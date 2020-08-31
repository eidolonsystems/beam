#ifndef BEAM_TCP_SOCKET_CONNECTION_HPP
#define BEAM_TCP_SOCKET_CONNECTION_HPP
#include <functional>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/TcpSocketOptions.hpp"

namespace Beam {
namespace Network {

  /** Implements a Connection using a TCP socket. */
  class TcpSocketConnection : private boost::noncopyable {
    public:
      ~TcpSocketConnection();

      void Close();

    private:
      friend class TcpSocketChannel;
      friend class TcpServerSocket;
      std::shared_ptr<Details::TcpSocketEntry> m_socket;
      IO::OpenState m_openState;

      TcpSocketConnection(std::shared_ptr<Details::TcpSocketEntry> socket);
      TcpSocketConnection(std::shared_ptr<Details::TcpSocketEntry> socket,
        const TcpSocketOptions& options, const IpAddress& address);
      TcpSocketConnection(std::shared_ptr<Details::TcpSocketEntry> socket,
        const TcpSocketOptions& options, const IpAddress& address,
        const IpAddress& interface);
      TcpSocketConnection(std::shared_ptr<Details::TcpSocketEntry> socket,
        const TcpSocketOptions& options,
        const std::vector<IpAddress>& addresses);
      TcpSocketConnection(std::shared_ptr<Details::TcpSocketEntry> socket,
        const TcpSocketOptions& options,
        const std::vector<IpAddress>& addresses, const IpAddress& interface);
      void Open(const TcpSocketOptions& options,
        const std::vector<IpAddress>& addresses,
        const boost::optional<IpAddress>& interface);
      void Shutdown();
  };

  inline TcpSocketConnection::~TcpSocketConnection() {
    Close();
  }

  inline void TcpSocketConnection::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline TcpSocketConnection::TcpSocketConnection(
    std::shared_ptr<Details::TcpSocketEntry> socket)
    : m_socket(std::move(socket)) {}

  inline TcpSocketConnection::TcpSocketConnection(
    std::shared_ptr<Details::TcpSocketEntry> socket,
    const TcpSocketOptions& options, const IpAddress& address)
    : TcpSocketConnection(std::move(socket), options,
        std::vector<IpAddress>{address}) {}

  inline TcpSocketConnection::TcpSocketConnection(
    std::shared_ptr<Details::TcpSocketEntry> socket,
    const TcpSocketOptions& options, const IpAddress& address,
    const IpAddress& interface)
    : TcpSocketConnection(std::move(socket), options,
        std::vector<IpAddress>{address}, interface) {}

  inline TcpSocketConnection::TcpSocketConnection(
      std::shared_ptr<Details::TcpSocketEntry> socket,
      const TcpSocketOptions& options, const std::vector<IpAddress>& addresses)
      : m_socket(std::move(socket)) {
    Open(options, addresses, boost::none);
  }

  inline TcpSocketConnection::TcpSocketConnection(
      std::shared_ptr<Details::TcpSocketEntry> socket,
      const TcpSocketOptions& options, const std::vector<IpAddress>& addresses,
      const IpAddress& interface)
      : m_socket(std::move(socket)) {
    Open(options, addresses, interface);
  }

  inline void TcpSocketConnection::Open(const TcpSocketOptions& options,
      const std::vector<IpAddress>& addresses,
      const boost::optional<IpAddress>& interface) {
    m_openState.SetOpening();
    auto errorCode = boost::system::error_code();
    for(auto& address : addresses) {
      errorCode.clear();
      auto resolver = boost::asio::ip::tcp::resolver(*m_socket->m_ioService);
      auto query = boost::asio::ip::tcp::resolver::query(address.GetHost(),
        std::to_string(address.GetPort()));
      auto end = boost::asio::ip::tcp::resolver::iterator();
      auto endpointIterator = resolver.resolve(query, errorCode);
      if(errorCode) {
        m_openState.SetOpenFailure(IO::ConnectException(errorCode.message()));
        Shutdown();
      }
      errorCode = boost::asio::error::host_not_found;
      while(errorCode && endpointIterator != end) {
        auto closeError = boost::system::error_code();
        m_socket->m_socket.close(closeError);
        if(interface) {
          auto localEndpoint = boost::asio::ip::tcp::endpoint(
            boost::asio::ip::address::from_string(interface->GetHost(),
            errorCode), interface->GetPort());
          if(errorCode) {
            m_openState.SetOpenFailure(
              IO::ConnectException(errorCode.message()));
            Shutdown();
          }
          m_socket->m_socket.open(boost::asio::ip::tcp::v4(), errorCode);
          if(errorCode) {
            m_openState.SetOpenFailure(
              IO::ConnectException(errorCode.message()));
            Shutdown();
          }
          m_socket->m_socket.bind(localEndpoint, errorCode);
          if(errorCode) {
            m_openState.SetOpenFailure(
              IO::ConnectException(errorCode.message()));
            Shutdown();
          }
        }
        m_socket->m_socket.connect(*endpointIterator, errorCode);
        ++endpointIterator;
      }
      if(!errorCode) {
        break;
      }
    }
    if(errorCode) {
      m_openState.SetOpenFailure(IO::ConnectException(errorCode.message()));
      Shutdown();
    }
    auto bufferSize = boost::asio::socket_base::send_buffer_size(
      options.m_writeBufferSize);
    m_socket->m_socket.set_option(bufferSize, errorCode);
    if(errorCode) {
      m_openState.SetOpenFailure(IO::ConnectException(errorCode.message()));
      Shutdown();
    }
    auto noDelay = boost::asio::ip::tcp::no_delay(options.m_noDelayEnabled);
    m_socket->m_socket.set_option(noDelay, errorCode);
    if(errorCode) {
      m_openState.SetOpenFailure(IO::ConnectException(errorCode.message()));
      Shutdown();
    }
    m_socket->m_isOpen = true;
    m_openState.SetOpen();
  }

  inline void TcpSocketConnection::Shutdown() {
    m_socket->Close();
    m_openState.SetClosed();
  }
}

  template<>
  struct ImplementsConcept<Network::TcpSocketConnection, IO::Connection> :
    std::true_type {};
}

#endif
