#include <fstream>
#include <iostream>
#include <boost/functional/factory.hpp>
#include <boost/lexical_cast.hpp>
#include <tclap/CmdLine.h>
#include <Viper/MySql/Connection.hpp>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/ServiceLocator/ApplicationDefinitions.hpp"
#include "Beam/ServiceLocator/AuthenticationServletAdapter.hpp"
#include "Beam/Services/ServiceProtocolServletContainer.hpp"
#include "Beam/Sql/MySqlConfig.hpp"
#include "Beam/Threading/LiveTimer.hpp"
#include "Beam/UidService/SqlUidDataStore.hpp"
#include "Beam/UidService/UidServlet.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/Streamable.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Serialization;
using namespace Beam::ServiceLocator;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace Beam::UidService;
using namespace boost;
using namespace boost::posix_time;
using namespace TCLAP;
using namespace Viper;

namespace {
  using UidServletContainer = ServiceProtocolServletContainer<
    MetaAuthenticationServletAdapter<
    MetaUidServlet<SqlUidDataStore<MySql::Connection>>,
    ApplicationServiceLocatorClient::Client*>, TcpServerSocket,
    BinarySender<SharedBuffer>, NullEncoder, std::shared_ptr<LiveTimer>>;

  struct ServerConnectionInitializer {
    std::string m_serviceName;
    IpAddress m_interface;
    std::vector<IpAddress> m_addresses;

    void Initialize(const YAML::Node& config);
  };

  void ServerConnectionInitializer::Initialize(const YAML::Node& config) {
    m_serviceName = Extract<std::string>(config, "service",
      UidService::SERVICE_NAME);
    m_interface = Extract<IpAddress>(config, "interface");
    auto addresses = std::vector<IpAddress>();
    addresses.push_back(m_interface);
    m_addresses = Extract<std::vector<IpAddress>>(config, "addresses",
      addresses);
  }
}

int main(int argc, const char** argv) {
  auto configFile = std::string();
  try {
    auto cmd = CmdLine("", ' ', "1.0-r" UID_SERVER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    auto configArg = ValueArg<std::string>("c", "config", "Configuration file",
      false, "config.yml", "path");
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() <<
      std::endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
  auto mySqlConfig = MySqlConfig();
  try {
    mySqlConfig = MySqlConfig::Parse(GetNode(config, "data_store"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'data_store': " << e.what() <<
      std::endl;
    return -1;
  }
  auto serverConnectionInitializer = ServerConnectionInitializer();
  try {
    serverConnectionInitializer.Initialize(GetNode(config, "server"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'server': " << e.what() << std::endl;
    return -1;
  }
  auto serviceLocatorClientConfig = ServiceLocatorClientConfig();
  try {
    serviceLocatorClientConfig = ServiceLocatorClientConfig::Parse(
      GetNode(config, "service_locator"));
  } catch(const std::exception& e) {
    std::cerr << "Error parsing section 'service_locator': " << e.what() <<
      std::endl;
    return -1;
  }
  auto socketThreadPool = SocketThreadPool();
  auto timerThreadPool = TimerThreadPool();
  auto serviceLocatorClient = ApplicationServiceLocatorClient();
  try {
    serviceLocatorClient.BuildSession(serviceLocatorClientConfig.m_username,
      serviceLocatorClientConfig.m_password,
      serviceLocatorClientConfig.m_address, Ref(socketThreadPool),
      Ref(timerThreadPool));
  } catch(const std::exception& e) {
    std::cerr << "Error logging in: " << e.what() << std::endl;
    return -1;
  }
  auto mySqlConnection = std::make_unique<MySql::Connection>(
    mySqlConfig.m_address.GetHost(), mySqlConfig.m_address.GetPort(),
    mySqlConfig.m_username, mySqlConfig.m_password, mySqlConfig.m_schema);
  auto server = boost::optional<UidServletContainer>();
  try {
    server.emplace(Initialize(serviceLocatorClient.Get(),
      Initialize(std::move(mySqlConnection))), Initialize(
      serverConnectionInitializer.m_interface, Ref(socketThreadPool)),
      std::bind(factory<std::shared_ptr<LiveTimer>>(), seconds(10),
      Ref(timerThreadPool)));
  } catch(const std::exception& e) {
    std::cerr << "Error opening server: " << e.what() << std::endl;
    return -1;
  }
  try {
    auto service = JsonObject();
    service["addresses"] = lexical_cast<std::string>(
      Stream(serverConnectionInitializer.m_addresses));
    serviceLocatorClient->Register(serverConnectionInitializer.m_serviceName,
      service);
  } catch(const std::exception& e) {
    std::cerr << "Error registering service: " << e.what() << std::endl;
    return -1;
  }
  WaitForKillEvent();
  return 0;
}
