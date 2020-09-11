#ifndef BEAM_UDP_SOCKET_CHANNEL_HPP
#define BEAM_UDP_SOCKET_CHANNEL_HPP
#include "Beam/IO/Channel.hpp"
#include "Beam/Network/SocketIdentifier.hpp"
#include "Beam/Network/UdpSocket.hpp"
#include "Beam/Network/UdpSocketConnection.hpp"
#include "Beam/Network/UdpSocketReader.hpp"
#include "Beam/Network/UdpSocketWriter.hpp"

namespace Beam {
namespace Network {

  /** Implements the Channel interface using a UDP socket. */
  class UdpSocketChannel {
    public:
      using Identifier = SocketIdentifier;
      using Connection = UdpSocketConnection;
      using Reader = UdpSocketReader;
      using Writer = UdpSocketWriter;

      /**
       * Constructs a UdpSocketChannel.
       * @param address The address to open.
       */
      UdpSocketChannel(const IpAddress& address);

      /**
       * Constructs a UdpSocketChannel.
       * @param address The address to open.
       * @param options The options to apply to the socket.
       */
      UdpSocketChannel(const IpAddress& address,
        const UdpSocketOptions& options);

      /**
       * Constructs a UdpSocketChannel.
       * @param address The address to open.
       * @param interface The interface to use.
       */
      UdpSocketChannel(const IpAddress& address, const IpAddress& interface);

      /**
       * Constructs a UdpSocketChannel.
       * @param address The address to open.
       * @param interface The interface to use.
       * @param options The options to apply to the socket.
       */
      UdpSocketChannel(const IpAddress& address, const IpAddress& interface,
        const UdpSocketOptions& options);

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      Identifier m_identifier;
      std::shared_ptr<UdpSocket> m_socket;
      Connection m_connection;
      Reader m_reader;
      Writer m_writer;

      UdpSocketChannel(const UdpSocketChannel&) = delete;
      UdpSocketChannel& operator =(const UdpSocketChannel&) = delete;
  };

  inline UdpSocketChannel::UdpSocketChannel(const IpAddress& address)
    : UdpSocketChannel(address, UdpSocketOptions()) {}

  inline UdpSocketChannel::UdpSocketChannel(const IpAddress& address,
    const UdpSocketOptions& options)
    : m_identifier(address),
      m_socket(std::make_shared<UdpSocket>(address, options)),
      m_connection(m_socket),
      m_reader(m_socket),
      m_writer(m_socket) {}

  inline UdpSocketChannel::UdpSocketChannel(const IpAddress& address,
    const IpAddress& interface)
    : UdpSocketChannel(address, interface, UdpSocketOptions()) {}

  inline UdpSocketChannel::UdpSocketChannel(const IpAddress& address,
    const IpAddress& interface, const UdpSocketOptions& options)
    : m_identifier(address),
      m_socket(std::make_shared<UdpSocket>(address, interface)),
      m_connection(m_socket),
      m_reader(m_socket),
      m_writer(m_socket) {}

  inline const UdpSocketChannel::Identifier&
      UdpSocketChannel::GetIdentifier() const {
    return m_identifier;
  }

  inline UdpSocketChannel::Connection& UdpSocketChannel::GetConnection() {
    return m_connection;
  }

  inline UdpSocketChannel::Reader& UdpSocketChannel::GetReader() {
    return m_reader;
  }

  inline UdpSocketChannel::Writer& UdpSocketChannel::GetWriter() {
    return m_writer;
  }
}

  template<>
  struct ImplementsConcept<Network::UdpSocketChannel, IO::Channel<
    Network::UdpSocketChannel::Identifier,
    Network::UdpSocketChannel::Connection,
    Network::UdpSocketChannel::Reader, Network::UdpSocketChannel::Writer>> :
    std::true_type {};
}

#endif
