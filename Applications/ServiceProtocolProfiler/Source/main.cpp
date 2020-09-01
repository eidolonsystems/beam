#include <fstream>
#include <iostream>
#include <boost/format.hpp>
#include <boost/functional/factory.hpp>
#include <boost/functional/value_factory.hpp>
#include <tclap/CmdLine.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/Codecs/SizeDeclarativeEncoder.hpp"
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/NotConnectedException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Utilities/ApplicationInterrupt.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/YamlConfig.hpp"
#include "ServiceProtocolProfiler/Services.hpp"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Routines;
using namespace Beam::Serialization;
using namespace Beam::Services;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace std;
using namespace TCLAP;

namespace {
  using ServiceEncoder = SizeDeclarativeEncoder<ZLibEncoder>;
  using ApplicationServerConnection = LocalServerConnection<SharedBuffer>;
  using ServerChannel = ApplicationServerConnection::Channel;
  using ApplicationServerServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<std::shared_ptr<ServerChannel>, BinarySender<SharedBuffer>,
    ServiceEncoder>, TriggerTimer>;
  using ClientChannel = LocalClientChannel<SharedBuffer>;
  using ApplicationClientServiceProtocolClient = ServiceProtocolClient<
    MessageProtocol<ClientChannel*, BinarySender<SharedBuffer>,
    ServiceEncoder>, TriggerTimer>;

  string OnEchoRequest(ApplicationServerServiceProtocolClient& client,
      string message) {
    return message;
  }

  void ServerLoop(ApplicationServerConnection& server) {
    RoutineHandlerGroup routines;
    while(true) {
      std::shared_ptr<ServerChannel> channel{server.Accept()};
      routines.Spawn(
        [=] {
          ApplicationServerServiceProtocolClient client(std::move(channel),
            Initialize());
          RegisterServiceProtocolProfilerServices(Store(client.GetSlots()));
          RegisterServiceProtocolProfilerMessages(Store(client.GetSlots()));
          EchoService::AddSlot(Store(client.GetSlots()),
            std::bind(OnEchoRequest, std::placeholders::_1,
            std::placeholders::_2));
          try {
            auto counter = 0;
            while(true) {
              auto message = client.ReadMessage();
              auto timestamp = microsec_clock::universal_time();
              ++counter;
              if(counter % 100000 == 0) {
                cout << boost::format("Server: %1% %2%\n") % &client %
                  timestamp << std::flush;
              }
            }
          } catch(const ServiceRequestException&) {
          } catch(const NotConnectedException&) {
          }
        });
    }
  }

  void ClientLoop(ApplicationServerConnection& server) {
    ClientChannel channel(string("client"), server);
    ApplicationClientServiceProtocolClient client(&channel, Initialize());
    RegisterServiceProtocolProfilerServices(Store(client.GetSlots()));
    RegisterServiceProtocolProfilerMessages(Store(client.GetSlots()));
    auto counter = 0;
    while(true) {
      auto timestamp = microsec_clock::universal_time();
      SendRecordMessage<EchoMessage>(client, timestamp, "hello world");
      ++counter;
      if(counter % 100000 == 0) {
        cout << boost::format("Client: %1% %2%\n") % &channel % timestamp <<
          std::flush;
      }
      Defer();
    }
    client.Close();
  }
}

int main(int argc, const char** argv) {
  string configFile;
  try {
    CmdLine cmd("", ' ', "1.0-r" SERVICE_PROTOCOL_PROFILER_VERSION
      "\nCopyright (C) 2020 Spire Trading Inc.");
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  auto config = Require(LoadFile, configFile);
  auto clientCount = Extract<int>(config, "clients", 0);
  if(clientCount == 0) {
    clientCount = static_cast<int>(boost::thread::hardware_concurrency());
  }
  ApplicationServerConnection server;
  RoutineHandlerGroup routines;
  routines.Spawn(
    [&] {
      ServerLoop(server);
    });
  for(auto i = 0; i < clientCount; ++i) {
    routines.Spawn(
      [&] {
        ClientLoop(server);
      });
  }
  routines.Wait();
  return 0;
}
