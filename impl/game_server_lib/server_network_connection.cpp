#include "server_network_connection.hpp"
#include "compression/compressor_interface.hpp"
#include "message.hpp"
#include <asio/error_code.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/write.hpp>
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
    , m_buffer { std::make_unique<ReceiveBuffer>() }
{
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

    m_thread = std::thread { [this]() {
        auto work_guard = asio::make_work_guard(m_IOContext);
        m_IOContext.run();
    } };
}

ServerNetworkConnection::~ServerNetworkConnection()
{
    try {
        m_logger.debug(
            "ServerNetworkConnection destructor called", { "network", "ServerNetworkConnection" });

        m_logger.debug("stop thread", { "stop thread", "ServerNetworkConnection" });
        m_IOContext.stop();
        m_thread.join();

        m_logger.debug("close sockets", { "network", "ServerNetworkConnection" });
        for (auto& s : m_sockets) {
            s->close();
        }

        m_logger.debug("clear socket pointers", { "network", "ServerNetworkConnection" });
        m_sockets.clear();
    } catch (std::exception const& e) {
        m_logger.fatal(e.what());
    }
}

void ServerNetworkConnection::setHandleIncomingMessageCallback(
    std::function<void(std::string const&, asio::ip::tcp::endpoint sendToEndpoint)> callback)
{
    m_handleInComingMessageCallback = callback;
}

void ServerNetworkConnection::update()
{
    std::lock_guard<std::mutex> lock { m_socketsMutex };

    // clean up sockets
    std::erase_if(m_sockets, [this](auto const& s) {
        if (!s->is_open()) {
            m_logger.info("remove closed socket", { "network", "ServerNetworkConnection" });
        }
        return !s->is_open();
    });
}

void ServerNetworkConnection::handleReceive(
    asio::ip::tcp::socket& socket, const asio::error_code& error, std::size_t length)
{
    NetworkHelpers::freeHandleReceive(
        error, length, socket, *m_buffer, m_logger, [this, &socket](std::string const& str) {
            std::string const uncompressed = m_compressor.decompress(str);
            handleMessage(uncompressed, socket.remote_endpoint());
            awaitNextMessageInternal(socket);
        });
}

void ServerNetworkConnection::awaitNextMessageInternal(asio::ip::tcp::socket& socket)
{
    socket.async_receive(asio::buffer(m_buffer->size.data(), m_buffer->size.size()),
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
    nlohmann::json const j = m;
    auto const str = m_compressor.compress(j.dump());

    for (auto& s : m_sockets) {
        if (!s || !s->is_open()) {
            m_logger.error(
                "send message with closed socket", { "network", "ServerNetworkConnection" });
            return;
        }

        sendStringTo(str, *s);
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
            sendStringTo(m_compressor.compress(j.dump()), *s);
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
    NetworkHelpers::freeSendString(str, socket, m_logger);
}

bool ServerNetworkConnection::isSocketOpenFor(const asio::ip::tcp::endpoint& endpoint) const
{
    std::lock_guard<std::mutex> lock { m_socketsMutex };
    for (auto const& s : m_sockets) {
        if (s->remote_endpoint().port() == endpoint.port()
            && s->remote_endpoint().address() == endpoint.address()) {
            if (s->is_open()) {
                return true;
            }
        }
    }
    return false;
}
