#include "client_network_connection.hpp"
#include "asio/executor_work_guard.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/udp.hpp"
#include "asio/socket_base.hpp"
#include "asio/write.hpp"
#include <message.hpp>
#include <network_properties.hpp>
#include <nlohmann.hpp>
#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

ClientNetworkConnection::ClientNetworkConnection(
    std::string const& ip, std::uint16_t serverPort, std::uint16_t clientPort)
    : m_ip { ip }
    , m_serverPort { serverPort }
    , m_clientPort { clientPort }
{
    // TODO pass in logger and use logger instead of cout
}

ClientNetworkConnection::~ClientNetworkConnection()
{
    m_socket->close();
    stopThread();

    m_socket.reset();
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

void ClientNetworkConnection::startReceive()
{
    std::cout << "start thread to process async tasks\n";
    m_workGuard = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(
        asio::make_work_guard(m_IOContext));
    m_thread = std::thread { [this]() { m_IOContext.run(); } };
}

void ClientNetworkConnection::handleReceive(
    const asio::error_code& /*error*/, std::size_t bytes_transferred)
{
    std::cout << "message received from '" << m_receivedFromEndpoint.address() << ":"
              << m_receivedFromEndpoint.port() << "'\nwith content\n";

    // Note that recv_buffer might be a long buffer, but we only use the first "bytes
    // transferred" bytes from it.

    // TODO think about adding a mutex here

    std::stringstream ss;
    ss.write(m_receiveBuffer.data(), bytes_transferred);
    auto const str = ss.str();
    std::cout << str << std::endl;
    // pass message up to be processed
    if (m_handleInComingMessageCallback) {
        m_handleInComingMessageCallback(str, m_receivedFromEndpoint);
    }
    m_socket->async_receive_from(asio::buffer(m_receiveBuffer), m_receivedFromEndpoint,
        std::bind(&ClientNetworkConnection::handleReceive, this, std::placeholders::_1,
            std::placeholders::_2));
}

void ClientNetworkConnection::sendInitialPing()
{
    Message m;
    m.type = MessageType::InitialPing;
    sendMessage(m);
}

void ClientNetworkConnection::sendAlivePing(int playerId)
{
    Message m;
    m.type = MessageType::StayAlivePing;
    m.playerId = playerId;
    sendMessage(m);
}

void ClientNetworkConnection::handle_send(std::shared_ptr<std::array<char, 1>> /*message*/,
    const asio::error_code& /*error*/, std::size_t /*bytes_transferred*/)
{
}

void ClientNetworkConnection::stopThread()
{
    m_IOContext.stop();
    m_thread.join();
}

void ClientNetworkConnection::sendMessage(const Message& m)
{
    if (!m_socket) {
        std::cerr << "socket not open\n";
        return;
    }

    nlohmann::json j = m;

    sendString(j.dump());
}

void ClientNetworkConnection::sendString(const std::string& str)
{
    asio::error_code error;
    auto size = m_socket->send_to(asio::buffer(str), m_sendToEndpoint, 0, error);

    // TODO think about adding a mutex here
    m_socket->async_receive_from(asio::buffer(m_receiveBuffer), m_receivedFromEndpoint,
        std::bind(&ClientNetworkConnection::handleReceive, this, std::placeholders::_1,
            std::placeholders::_2));
    std::cout << "message sent with size:" << size << "\n";
    std::cout << str << std::endl;
    std::cout << error.message() << std::endl;
}

void ClientNetworkConnection::setHandleIncomingMessageCallback(
    std::function<void(std::string const&, asio::ip::udp::endpoint sendToEndpoint)> callback)
{
    m_handleInComingMessageCallback = callback;
}
