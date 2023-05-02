#include "client_network_connection.hpp"
#include "asio/ip/udp.hpp"
#include "asio/socket_base.hpp"
#include "asio/write.hpp"
#include <network_properties.hpp>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

ClientNetworkConnection::ClientNetworkConnection(
    std::string const& ip, std::uint16_t serverPort, std::uint16_t clientPort)
    : m_ip { ip }
    , m_serverPort { serverPort }
    , m_clientPort { clientPort }
{
    // TODO load default from config file
    // TODO pass in logger and use logger instead of cout
    // TODO use separate thread so main thread is not blocked
}

void ClientNetworkConnection::establishConnection()
{
    asio::ip::udp::resolver resolver(m_IOContext);
    m_socket = std::make_unique<asio::ip::udp::socket>(m_IOContext,
        asio::ip::udp::endpoint { NetworkProperties::NetworkProtocolType(), m_clientPort });

    m_sendToEndpoint = *resolver
                            .resolve(NetworkProperties::NetworkProtocolType(), m_ip,
                                std::to_string(m_serverPort))
                            .begin();

    startReceive();
}

void ClientNetworkConnection::sendPing()
{
    if (!m_socket) {
        std::cerr << "socket not open\n";
        return;
    }

    // send ping
    std::cout << "send ping to '" << m_sendToEndpoint.address() << " : " << m_sendToEndpoint.port()
              << "'\n";
    std::array<char, 1> send_buf = { { 0 } };
    asio::error_code error;
    auto size = m_socket->send_to(asio::buffer(send_buf), m_sendToEndpoint, 0, error);

    m_socket->async_receive_from(asio::buffer(recv_buffer_), m_receivedFromEndpoint,
        std::bind(&ClientNetworkConnection::handleReceive, this, std::placeholders::_1,
            std::placeholders::_2));
    std::cout << "ping sent '" << size << "'\n";
    std::cout << error.message() << std::endl;
}

void ClientNetworkConnection::startReceive()
{
    std::cout << "start thread to process async tasks\n";

    m_thread = std::thread { [this]() {
        while (true) {
            if (m_stopThread.load()) {
                std::cerr << "thread stopped\n";
                break;
            }

            m_IOContext.run();
            m_IOContext.restart();
        }
    } };
}

void ClientNetworkConnection::handleReceive(
    const asio::error_code& /*error*/, std::size_t bytes_transferred)
{
    std::cout << "message received from '" << m_receivedFromEndpoint.address() << ":"
              << m_receivedFromEndpoint.port() << "'\n";
    // Note that recv_buffer might be a long buffer, but we only use the first "bytes transferred"
    // bytes from it.
    std::string buf;
    //    buf.resize(bytes_transferred);
    //    m_receive_buffer.sgetn(&buf[0], buf.size());
    //    std::cout.write(recv_buffer_.str(), bytes_transferred);
    std::cout << buf << "\n\n";
}

void ClientNetworkConnection::handle_send(std::shared_ptr<std::array<char, 1>> /*message*/,
    const asio::error_code& /*error*/, std::size_t /*bytes_transferred*/)
{
}
void ClientNetworkConnection::disconnect() { }
ClientNetworkConnection::~ClientNetworkConnection()
{
    disconnect();
    stopThread();
    m_socket.reset();
}
void ClientNetworkConnection::stopThread()
{
    m_stopThread.store(true);
    m_thread.join();
}
