#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServicesTests/ServicesTests.hpp"
#include "Beam/SignalHandling/NullSlot.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Services::Tests;
using namespace Beam::SignalHandling;
using namespace Beam::Threading;
using namespace boost;

namespace {
  LoginServiceResult AcceptLoginRequest(
      TestServiceProtocolServer::ServiceProtocolClient& client,
      const std::string& username, const std::string& password,
      bool& receivedRequest) {
    auto account = DirectoryEntry::MakeAccount(0, "account");
    receivedRequest = true;
    return LoginServiceResult(account, "sessionid");
  }

  LoginServiceResult RejectLoginRequest(
      TestServiceProtocolServer::ServiceProtocolClient& client,
      const std::string& username, const std::string& password,
      bool& receivedRequest) {
    receivedRequest = true;
    throw ServiceRequestException();
  }

  struct Fixture {
    using TestServiceLocatorClient = ServiceLocatorClient<
      TestServiceProtocolClientBuilder>;
    boost::optional<TestServiceProtocolServer> m_protocolServer;
    boost::optional<TestServiceLocatorClient> m_serviceClient;
    std::vector<TestServiceProtocolClientBuilder::Channel*> m_clientChannels;

    Fixture() {
      auto serverConnection = std::make_shared<TestServerConnection>();
      m_protocolServer.emplace(serverConnection,
        factory<std::unique_ptr<TriggerTimer>>(), NullSlot(), NullSlot());
      m_protocolServer->Open();
      RegisterServiceLocatorServices(Store(m_protocolServer->GetSlots()));
      RegisterServiceLocatorMessages(Store(m_protocolServer->GetSlots()));
      auto builder = TestServiceProtocolClientBuilder(
        [=] {
          auto channel = std::make_unique<
            TestServiceProtocolClientBuilder::Channel>("test",
            Ref(*serverConnection));
          m_clientChannels.push_back(channel.get());
          return channel;
        }, factory<std::unique_ptr<TestServiceProtocolClientBuilder::Timer>>());
      m_serviceClient.emplace(builder);
    }
  };
}

TEST_SUITE("ServiceLocatorClient") {
  TEST_CASE_FIXTURE(Fixture, "login_accepted") {
    auto receivedRequest = false;
    LoginService::AddSlot(Store(m_protocolServer->GetSlots()), std::bind(
      AcceptLoginRequest, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3, std::ref(receivedRequest)));
    m_serviceClient->SetCredentials("account", "password");
    REQUIRE_NOTHROW(m_serviceClient->Open());
    REQUIRE(receivedRequest);
    REQUIRE(m_serviceClient->GetAccount().m_name == "account");
    REQUIRE(m_serviceClient->GetSessionId() == "sessionid");
  }

  TEST_CASE_FIXTURE(Fixture, "login_rejected") {
    auto receivedRequest = false;
    LoginService::AddSlot(Store(m_protocolServer->GetSlots()), std::bind(
      RejectLoginRequest, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3, std::ref(receivedRequest)));
    m_serviceClient->SetCredentials("account", "password");
    REQUIRE_THROWS_AS(m_serviceClient->Open(), ServiceRequestException);
  }

  TEST_CASE_FIXTURE(Fixture, "monitor_accounts") {
    auto receivedRequest = false;
    auto receivedUnmonitor = Async<void>();
    LoginService::AddSlot(Store(m_protocolServer->GetSlots()), std::bind(
      AcceptLoginRequest, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3, std::ref(receivedRequest)));
    UnmonitorAccountsService::AddSlot(Store(m_protocolServer->GetSlots()),
      [&] (auto& client, int dummy) {
        receivedUnmonitor.GetEval().SetResult();
      });
    auto testAccounts = std::vector<DirectoryEntry>();
    testAccounts.push_back(DirectoryEntry::MakeAccount(123, "accountA"));
    testAccounts.push_back(DirectoryEntry::MakeAccount(124, "accountB"));
    testAccounts.push_back(DirectoryEntry::MakeAccount(125, "accountC"));
    auto serverSideClient =
      static_cast<TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    MonitorAccountsService::AddSlot(Store(m_protocolServer->GetSlots()),
      [&] (auto& client, int dummy) {
        serverSideClient = &client;
        return testAccounts;
      });
    m_serviceClient->SetCredentials("account", "password");
    REQUIRE_NOTHROW(m_serviceClient->Open());
    auto accountQueue = std::make_shared<Queue<AccountUpdate>>();
    m_serviceClient->MonitorAccounts(accountQueue);
    auto update = accountQueue->Top();
    accountQueue->Pop();
    REQUIRE(update ==
      AccountUpdate{testAccounts[0], AccountUpdate::Type::ADDED});
    update = accountQueue->Top();
    accountQueue->Pop();
    REQUIRE(update ==
      AccountUpdate{testAccounts[1], AccountUpdate::Type::ADDED});
    update = accountQueue->Top();
    accountQueue->Pop();
    REQUIRE(update ==
      AccountUpdate{testAccounts[2], AccountUpdate::Type::ADDED});
    SendRecordMessage<AccountUpdateMessage>(*serverSideClient,
      AccountUpdate{testAccounts[0], AccountUpdate::Type::DELETED});
    update = accountQueue->Top();
    REQUIRE(update ==
      AccountUpdate{testAccounts[0], AccountUpdate::Type::DELETED});
    auto duplicateQueue = std::make_shared<Queue<AccountUpdate>>();
    m_serviceClient->MonitorAccounts(duplicateQueue);
    update = duplicateQueue->Top();
    duplicateQueue->Pop();
    REQUIRE(update ==
      AccountUpdate{testAccounts[1], AccountUpdate::Type::ADDED});
    update = duplicateQueue->Top();
    duplicateQueue->Pop();
    REQUIRE(update ==
      AccountUpdate{testAccounts[2], AccountUpdate::Type::ADDED});
    accountQueue = nullptr;
    duplicateQueue = nullptr;
    receivedRequest = false;
    SendRecordMessage<AccountUpdateMessage>(*serverSideClient,
      AccountUpdate{testAccounts[1], AccountUpdate::Type::DELETED});
    REQUIRE_NOTHROW(receivedUnmonitor.Get());
  }

  TEST_CASE_FIXTURE(Fixture, "monitor_accounts_reconnect") {
    auto receivedRequest = false;
    LoginService::AddSlot(Store(m_protocolServer->GetSlots()), std::bind(
      AcceptLoginRequest, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3, std::ref(receivedRequest)));
    auto testAccounts = std::vector<DirectoryEntry>();
    testAccounts.push_back(DirectoryEntry::MakeAccount(123, "accountA"));
    testAccounts.push_back(DirectoryEntry::MakeAccount(124, "accountB"));
    testAccounts.push_back(DirectoryEntry::MakeAccount(125, "accountC"));
    auto serverSideClient =
      static_cast<TestServiceProtocolServer::ServiceProtocolClient*>(nullptr);
    MonitorAccountsService::AddSlot(Store(m_protocolServer->GetSlots()),
      [&] (auto& client, int dummy) {
        serverSideClient = &client;
        return testAccounts;
      });
    m_serviceClient->SetCredentials("account", "password");
    REQUIRE_NOTHROW(m_serviceClient->Open());
    auto accountQueue = std::make_shared<Queue<AccountUpdate>>();
    m_serviceClient->MonitorAccounts(accountQueue);
    for(auto i = std::size_t(0); i != testAccounts.size(); ++i) {
      accountQueue->Top();
      accountQueue->Pop();
    }
    testAccounts.push_back(DirectoryEntry::MakeAccount(135, "accountD"));
    m_clientChannels.back()->GetConnection().Close();
    auto recoveredAccount = accountQueue->Top();
    accountQueue->Pop();
    REQUIRE(recoveredAccount ==
      AccountUpdate{testAccounts.back(), AccountUpdate::Type::ADDED});
    m_serviceClient->Close();
    REQUIRE_THROWS_AS(accountQueue->Top(), PipeBrokenException);
  }
}
