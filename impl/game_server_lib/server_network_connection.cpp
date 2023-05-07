
#include "server_network_connection.hpp"
#include "message.hpp"
#include <network_properties.hpp>
#include <iostream>
#include <sstream>

namespace {
std::string make_daytime_string()
{
    using namespace std; // For time_t, time and ctime;
    time_t now = time(nullptr);
    return ctime(&now);
}
} // namespace

ServerNetworkConnection::ServerNetworkConnection()
    : m_socket { std::make_unique<asio::ip::udp::socket>(m_IOContext,
        asio::ip::udp::endpoint(
            NetworkProperties::NetworkProtocolType(), NetworkProperties::DefaultServerPort())) }
{
    // TODO pass in logger and use logger instead of cout
    std::cout << "start thread to handle async tasks\n";

    awaitNextMessageInternal();

    m_workGuard = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(
        asio::make_work_guard(m_IOContext));
    m_thread = std::thread { [this]() {
        while (true) {
            m_IOContext.run();
        }
    } };
}

ServerNetworkConnection::~ServerNetworkConnection()
{
    m_IOContext.stop();
    m_socket->close();
    m_thread.join();

    m_socket.reset();
}

void ServerNetworkConnection::setHandleIncomingMessageCallback(
    std::function<void(std::string const&, asio::ip::udp::endpoint sendToEndpoint)> callback)
{
    m_handleInComingMessageCallback = callback;
}

void ServerNetworkConnection::update() { }

void ServerNetworkConnection::handleReceive(const asio::error_code& /*error*/, std::size_t length)
{
    std::stringstream ss;
    ss.write(m_receiveBuffer.data(), length);
    auto const str = ss.str();
    handleMessage(str, m_remote_endpoint);
    awaitNextMessageInternal();
}

void ServerNetworkConnection::awaitNextMessageInternal()
{
    m_socket->async_receive_from(asio::buffer(m_receiveBuffer), m_remote_endpoint,
        std::bind(&ServerNetworkConnection::handleReceive, this, std::placeholders::_1,
            std::placeholders::_2));
}

void ServerNetworkConnection::handleMessage(
    std::string const& str, asio::ip::udp::endpoint endpoint)
{
    std::cout << "received message from endpoint: " << endpoint.address() << ":" << endpoint.port()
              << "'\n";
    // generic message handling
    if (m_handleInComingMessageCallback) {
        m_handleInComingMessageCallback(str, endpoint);
    }
}

void ServerNetworkConnection::sendMessage(const Message& m, asio::ip::udp::endpoint sendToEndpoint)
{
    if (!m_socket) {
        std::cerr << "socket not open\n";
        return;
    }

    nlohmann::json j = m;
    sendStringTo(j.dump(), sendToEndpoint);
    std::cout << "sending message\n" << j.dump() << std::endl;
}

void ServerNetworkConnection::sendStringTo(
    const std::string& str, asio::ip::udp::endpoint sendToEndpoint)
{
    asio::error_code error;
    auto size = m_socket->send_to(asio::buffer(str), sendToEndpoint, 0, error);
    std::cout << "ping sent '" << size << "'\n";
    std::cout << error.message() << std::endl;
}
