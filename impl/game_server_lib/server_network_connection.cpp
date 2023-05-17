#include "server_network_connection.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/write.hpp"
#include "compression/compressor_interface.hpp"
#include "message.hpp"
#include <network_helpers.hpp>
#include <network_properties.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>

ServerNetworkConnection::ServerNetworkConnection(
    CompressorInterface& compressor, jt::LoggerInterface& logger)
    : m_compressor { compressor }
    , m_logger { logger }
    , m_acceptor { m_IOContext,
        asio::ip::tcp::endpoint {
            NetworkProperties::NetworkProtocolType(), NetworkProperties::DefaultServerPort() } }
{

    m_socket = std::make_unique<asio::ip::tcp::socket>(m_IOContext);
    // TODO make the socket accept more than one connection
    m_acceptor.accept(*m_socket);
    m_logger.info("incoming connection accepted", { "network", "ServerNetworkConnection" });
    awaitNextMessageInternal();

    startProcessing();
}

void ServerNetworkConnection::startProcessing()
{
    m_logger.info("start thread to process async tasks", { "network", "ServerNetworkConnection" });
    m_workGuard = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(
        asio::make_work_guard(m_IOContext));
    m_thread = std::thread { [this]() { m_IOContext.run(); } };
}

ServerNetworkConnection::~ServerNetworkConnection()
{
    m_workGuard.reset();
    m_IOContext.stop();
    m_socket->close();
    m_thread.join();

    m_socket.reset();
}

void ServerNetworkConnection::setHandleIncomingMessageCallback(
    std::function<void(std::string const&, asio::ip::tcp::endpoint sendToEndpoint)> callback)
{
    m_handleInComingMessageCallback = callback;
}

void ServerNetworkConnection::update() { }

void ServerNetworkConnection::handleReceive(const asio::error_code& error, std::size_t length)
{
    NetworkHelpers::freeHandleReceive(
        error, length, *m_socket, m_buffer, m_logger, [this](std::string const& str) {
            std::string const uncompressed = m_compressor.decompress(str);
            handleMessage(uncompressed, m_remote_endpoint);
            awaitNextMessageInternal();
        });
}

void ServerNetworkConnection::awaitNextMessageInternal()
{
    m_socket->async_receive(
        asio::buffer(m_buffer.size.data(), m_buffer.size.size()), [this](auto ec, auto len) {
            std::unique_lock<std::mutex> lock { m_bufferMutex };
            handleReceive(ec, len);
        });
}

void ServerNetworkConnection::handleMessage(
    std::string const& str, asio::ip::tcp::endpoint endpoint)
{
    std::stringstream ss;
    ss << "received message from endpoint: " << endpoint.address() << ":" << endpoint.port()
       << "'\n";
    m_logger.debug(ss.str(), { "network", "ServerNetworkConnection" });

    // generic message handling
    if (m_handleInComingMessageCallback) {
        m_handleInComingMessageCallback(str, endpoint);
    }
}

void ServerNetworkConnection::sendMessage(const Message& m, asio::ip::tcp::endpoint sendToEndpoint)
{
    if (!m_socket || !m_socket->is_open()) {
        m_logger.error("send message with closed socket", { "network", "ServerNetworkConnection" });
        return;
    }

    nlohmann::json j = m;
    sendStringTo(j.dump(), sendToEndpoint);
}

void ServerNetworkConnection::sendStringTo(
    std::string const& str, asio::ip::tcp::endpoint /*sendToEndpoint*/)
{
    std::string const compressed = m_compressor.compress(str);
    NetworkHelpers::freeSendString(compressed, *m_socket, m_logger);
}
