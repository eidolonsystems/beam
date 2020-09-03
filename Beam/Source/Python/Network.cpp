#include "Beam/Python/Network.hpp"
#include <boost/lexical_cast.hpp>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include "Beam/IO/VirtualChannel.hpp"
#include "Beam/IO/VirtualChannelIdentifier.hpp"
#include "Beam/IO/VirtualServerConnection.hpp"
#include "Beam/IO/WrapperChannel.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/SocketIdentifier.hpp"
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Network/TcpSocketOptions.hpp"
#include "Beam/Python/Beam.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace boost;
using namespace pybind11;

void Beam::Python::ExportIpAddress(pybind11::module& module) {
  class_<IpAddress>(module, "IpAddress")
    .def(init())
    .def(init<std::string, unsigned short>())
    .def(init<const IpAddress&>())
    .def("__str__", &lexical_cast<std::string, IpAddress>)
    .def_property_readonly("host", &IpAddress::GetHost)
    .def_property_readonly("port", &IpAddress::GetPort)
    .def(self == self)
    .def(self != self);
}

void Beam::Python::ExportNetwork(pybind11::module& module) {
  auto submodule = module.def_submodule("network");
  ExportIpAddress(submodule);
  ExportSocketIdentifier(submodule);
  ExportTcpServerSocket(submodule);
  ExportTcpSocketChannel(submodule);
  ExportTcpSocketConnection(submodule);
  ExportTcpSocketOptions(submodule);
  ExportTcpSocketReader(submodule);
  ExportTcpSocketWriter(submodule);
  register_exception<SocketException>(submodule, "SocketException",
    GetIOException().ptr());
}

void Beam::Python::ExportSocketIdentifier(pybind11::module& module) {
  class_<WrapperChannelIdentifier<SocketIdentifier>,
    VirtualChannelIdentifier>(module, "SocketIdentifier")
    .def(init(
      [] {
        return new WrapperChannelIdentifier<SocketIdentifier>(
          SocketIdentifier());
      }))
    .def(init(
      [] (const IpAddress& address) {
        return new WrapperChannelIdentifier<SocketIdentifier>(
          SocketIdentifier(address));
      }))
    .def_property_readonly("address",
      [] (WrapperChannelIdentifier<SocketIdentifier>& self) {
        return self.GetBase().GetAddress();
      });
}

void Beam::Python::ExportTcpServerSocket(pybind11::module& module) {
  class_<WrapperServerConnection<std::unique_ptr<TcpServerSocket>>,
    VirtualServerConnection>(module, "TcpServerSocket")
    .def("accept",
      &WrapperServerConnection<std::unique_ptr<TcpServerSocket>>::Accept,
      call_guard<GilRelease>())
    .def("close",
      &WrapperServerConnection<std::unique_ptr<TcpServerSocket>>::Close,
      call_guard<GilRelease>());
}

void Beam::Python::ExportTcpSocketChannel(pybind11::module& module) {
  class_<WrapperVirtualChannel<std::unique_ptr<TcpSocketChannel>>,
      VirtualChannel>(module, "TcpSocketChannel")
    .def(init(
      [] (const IpAddress& address) {
        auto channel = std::make_unique<TcpSocketChannel>(address,
          Ref(*GetSocketThreadPool()));
        return new WrapperVirtualChannel<std::unique_ptr<TcpSocketChannel>>(
          std::move(channel));
      }), call_guard<GilRelease>())
    .def(init(
      [] (const IpAddress& address, const TcpSocketOptions& options) {
        auto channel = std::make_unique<TcpSocketChannel>(address, options,
          Ref(*GetSocketThreadPool()));
        return new WrapperVirtualChannel<std::unique_ptr<TcpSocketChannel>>(
          std::move(channel));
      }), call_guard<GilRelease>())
    .def(init(
      [] (const IpAddress& address, const IpAddress& interface) {
        auto channel = std::make_unique<TcpSocketChannel>(address, interface,
          Ref(*GetSocketThreadPool()));
        return new WrapperVirtualChannel<std::unique_ptr<TcpSocketChannel>>(
          std::move(channel));
      }), call_guard<GilRelease>())
    .def(init(
      [] (const IpAddress& address, const IpAddress& interface,
          const TcpSocketOptions& options) {
        auto channel = std::make_unique<TcpSocketChannel>(address, interface,
          options, Ref(*GetSocketThreadPool()));
        return new WrapperVirtualChannel<std::unique_ptr<TcpSocketChannel>>(
          std::move(channel));
      }), call_guard<GilRelease>())
    .def(init(
      [] (const std::vector<IpAddress>& addresses) {
        auto channel = std::make_unique<TcpSocketChannel>(addresses,
          Ref(*GetSocketThreadPool()));
        return new WrapperVirtualChannel<std::unique_ptr<TcpSocketChannel>>(
          std::move(channel));
      }), call_guard<GilRelease>())
    .def(init(
      [] (const std::vector<IpAddress>& addresses,
          const TcpSocketOptions& options) {
        auto channel = std::make_unique<TcpSocketChannel>(addresses, options,
          Ref(*GetSocketThreadPool()));
        return new WrapperVirtualChannel<std::unique_ptr<TcpSocketChannel>>(
          std::move(channel));
      }), call_guard<GilRelease>())
    .def(init(
      [] (const std::vector<IpAddress>& addresses, const IpAddress& interface) {
        auto channel = std::make_unique<TcpSocketChannel>(addresses, interface,
          Ref(*GetSocketThreadPool()));
        return new WrapperVirtualChannel<std::unique_ptr<TcpSocketChannel>>(
          std::move(channel));
      }), call_guard<GilRelease>())
    .def(init(
      [] (const std::vector<IpAddress>& addresses, const IpAddress& interface,
          const TcpSocketOptions& options) {
        auto channel = std::make_unique<TcpSocketChannel>(addresses, interface,
          options, Ref(*GetSocketThreadPool()));
        return new WrapperVirtualChannel<std::unique_ptr<TcpSocketChannel>>(
          std::move(channel));
      }), call_guard<GilRelease>());
}

void Beam::Python::ExportTcpSocketConnection(pybind11::module& module) {
  class_<WrapperConnection<TcpSocketConnection*>, VirtualConnection>(module,
    "TcpSocketConnection");
}

void Beam::Python::ExportTcpSocketOptions(pybind11::module& module) {
  class_<TcpSocketOptions>(module, "TcpSocketOptions")
    .def(init<>())
    .def(init<const TcpSocketOptions&>())
    .def_readwrite("no_delay_enabled", &TcpSocketOptions::m_noDelayEnabled)
    .def_readwrite("write_buffer_size", &TcpSocketOptions::m_writeBufferSize);
}

void Beam::Python::ExportTcpSocketReader(pybind11::module& module) {
  class_<WrapperReader<TcpSocketReader*>, VirtualReader>(module,
    "TcpSocketReader");
}

void Beam::Python::ExportTcpSocketWriter(pybind11::module& module) {
  class_<WrapperWriter<TcpSocketWriter*>, VirtualWriter>(module,
    "TcpSocketWriter");
}
