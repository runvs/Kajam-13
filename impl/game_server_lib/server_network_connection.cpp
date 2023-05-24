#include "server_network_connection.hpp"
#include "asio/error_code.hpp"
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
#include <string>

ServerNetworkConnection::ServerNetworkConnection(
    CompressorInterface& compressor, jt::LoggerInterface& logger)
    : m_compressor { compressor }
    , m_logger { logger }
    , m_acceptor { m_IOContext,
        asio::ip::tcp::endpoint {
            NetworkProperties::NetworkProtocolType(), NetworkProperties::DefaultServerPort() } }
{

    // TODO make the socket accept more than one connection in an async way
    //    m_acceptor.accept(*m_socket);
    awaitNextAccept();

    startProcessing();
}

void ServerNetworkConnection::handleAccept(asio::error_code ec, asio::ip::tcp::socket&& socket)
{
    if (ec) {
        m_logger.error(
            "Accept connection error:" + ec.message(), { "network", "ServerNetworkConnection" });
    }
    std::lock_guard<std::mutex> lock { m_socketsMutex };
    m_sockets.emplace_back(std::make_unique<asio::ip::tcp::socket>(std::move(socket)));

    m_logger.info("incoming connection accepted from "
            + m_sockets.back()->remote_endpoint().address().to_string() + ":"
            + std::to_string(m_sockets.back()->remote_endpoint().port()),
        { "network", "ServerNetworkConnection" });
    awaitNextMessageInternal(*m_sockets.back());
    awaitNextAccept();
}
void ServerNetworkConnection::awaitNextAccept()
{
    m_acceptor.async_accept([this](asio::error_code ec, asio::ip::tcp::socket&& socket) {
        handleAccept(ec, std::move(socket));
    });
}

void ServerNetworkConnection::startProcessing()
{
    m_logger.info("start thread to process async tasks", { "network", "ServerNetworkConnection" });
    // TODO check if work guard can be instantiated inside thread

    m_thread = std::thread { [this]() {
        auto work_guard = asio::make_work_guard(m_IOContext);
        m_IOContext.run();
    } };
}

ServerNetworkConnection::~ServerNetworkConnection()
{
    m_IOContext.stop();
    for (auto& s : m_sockets) {
        s->close();
    }
    m_thread.join();

    m_sockets.clear();
}

void ServerNetworkConnection::setHandleIncomingMessageCallback(
    std::function<void(std::string const&, asio::ip::tcp::endpoint sendToEndpoint)> callback)
{
    m_handleInComingMessageCallback = callback;
}

void ServerNetworkConnection::update() { }

void ServerNetworkConnection::handleReceive(
    asio::ip::tcp::socket& socket, const asio::error_code& error, std::size_t length)
{
    NetworkHelpers::freeHandleReceive(
        error, length, socket, m_buffer, m_logger, [this, &socket](std::string const& str) {
            std::string const uncompressed = m_compressor.decompress(str);
            handleMessage(uncompressed, socket.remote_endpoint());
            awaitNextMessageInternal(socket);
        });
}

void ServerNetworkConnection::awaitNextMessageInternal(asio::ip::tcp::socket& socket)
{
    // TODO make async_read
    socket.async_receive(asio::buffer(m_buffer.size.data(), m_buffer.size.size()),
        [this, &socket](auto ec, auto len) { handleReceive(socket, ec, len); });
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

void ServerNetworkConnection::sendMessageToAll(const Message& m)
{
    for (auto& s : m_sockets) {
        if (!s || !s->is_open()) {
            m_logger.error(
                "send message with closed socket", { "network", "ServerNetworkConnection" });
            return;
        }

        nlohmann::json j = m;
        sendStringTo(j.dump(), *s);
    }
}

void ServerNetworkConnection::sendMessageToOne(
    const Message& m, const asio::ip::tcp::endpoint& endpoint)
{
    bool found = false;
    for (auto& s : m_sockets) {
        if (endpoint == s->remote_endpoint()) {
            if (!s || !s->is_open()) {
                m_logger.error(
                    "send message with closed socket", { "network", "ServerNetworkConnection" });
                return;
            }

            nlohmann::json j = m;
            sendStringTo(j.dump(), *s);
            found = true;
            break;
        }
    }
    if (!found) {
        m_logger.warning(
            "sendMessageToOne with invalid endpoint", { "network", "ServerNetworkConnection" });
    }
}

void ServerNetworkConnection::sendStringTo(std::string const& str, asio::ip::tcp::socket& socket)
{
    std::string const compressed = m_compressor.compress(str);
    NetworkHelpers::freeSendString(std::move(compressed), socket, m_logger);
}
