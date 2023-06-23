#include "client_network_connection.hpp"
#include <asio/connect.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/read.hpp>
#include <game_properties.hpp>
#include <log/logger_interface.hpp>
#include <message.hpp>
#include <network_helpers.hpp>
#include <network_properties.hpp>
#include <nlohmann.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

ClientNetworkConnection::ClientNetworkConnection(std::string const& ip, std::uint16_t serverPort,
    std::uint16_t clientPort, jt::LoggerInterface& logger,
    std::shared_ptr<CompressorInterface> compressor)
    : m_ip { ip }
    , m_serverPort { serverPort }
    , m_clientPort { clientPort }
    , m_logger { logger }
    , m_compressor { compressor }
    , m_buffer { std::make_unique<ReceiveBuffer>() }
{
}

ClientNetworkConnection::~ClientNetworkConnection()
{
    m_logger.debug(
        "ClientNetworkConnection destructor called", { "network", "ClientNetworkConnection" });
    m_logger.verbose("close socket", { "network", "ClientNetworkConnection" });
    m_socket->close();
    m_logger.verbose("stop thread", { "stop thread", "ClientNetworkConnection" });
    stopThread();
    m_logger.verbose("reset socket pointer", { "network", "ClientNetworkConnection" });
    m_socket.reset();
}

void ClientNetworkConnection::establishConnection()
{
    if (m_socket) {
        m_logger.error("connection was already established, discarding second connection attempt",
            { "network", "ClientNetworkConnection" });
        return;
    }

    asio::ip::tcp::resolver resolver(m_IOContext);
    m_socket = std::make_unique<asio::ip::tcp::socket>(m_IOContext,
        asio::ip::tcp::endpoint { NetworkProperties::NetworkProtocolType(), m_clientPort });
    auto endpoints = resolver.resolve(
        NetworkProperties::NetworkProtocolType(), m_ip, std::to_string(m_serverPort));
    m_sendToEndpoint = asio::connect(*m_socket, endpoints);

    startProcessing();
}

void ClientNetworkConnection::startProcessing()
{
    m_logger.debug("start thread to process async tasks", { "network", "ClientNetworkConnection" });
    m_thread = std::thread { [this]() {
        auto work_guard = asio::make_work_guard(m_IOContext);
        m_IOContext.run();
    } };
    // start listening for messages
    if (!m_alreadyReceiving.exchange(true)) {
        awaitNextMessage();
    }
}

namespace {

} // namespace

void ClientNetworkConnection::handleReceive(
    asio::error_code const& error, std::size_t bytes_transferred)
{
    NetworkHelpers::freeHandleReceive(
        error, bytes_transferred, *m_socket, *m_buffer, m_logger, [this](std::string const& str) {
            std::string const uncompressed = m_compressor->decompress(str);

            std::stringstream ss_log;
            ss_log << "message received from '" << m_receivedFromEndpoint.address() << ":"
                   << m_receivedFromEndpoint.port() << "'\nwith content\n"
                   << uncompressed;
            m_logger.debug(ss_log.str(), { "network", "ClientNetworkConnection" });

            // pass message up to be processed
            if (m_handleInComingMessageCallback) {
                m_handleInComingMessageCallback(uncompressed, m_receivedFromEndpoint);
            }

            // queue next handler so the next message will be processed as well
            awaitNextMessage();
        });
}

void ClientNetworkConnection::awaitNextMessage()
{
    asio::async_read(*m_socket, asio::buffer(m_buffer->size.data(), m_buffer->size.size()),
        [this](auto ec, auto len) { handleReceive(ec, len); });
}

void ClientNetworkConnection::sendInitialPing()
{
    m_logger.debug("send initial ping", { "network", "ClientNetworkConnection" });
    Message m;
    m.type = MessageType::InitialPing;
    sendMessage(m);
}

void ClientNetworkConnection::stopThread()
{
    m_IOContext.stop();
    m_thread.join();
}

void ClientNetworkConnection::sendMessage(const Message& m)
{

    if (!m_socket) {
        m_logger.warning(
            "send message called with nullptr socket", { "network", "ClientNetworkConnection" });
        return;
    }

    nlohmann::json j = m;
    sendString(j.dump());
}

void ClientNetworkConnection::sendString(const std::string& str)
{
    std::string compressed = m_compressor->compress(str);
    NetworkHelpers::freeSendString(compressed, *m_socket, m_logger);
}

void ClientNetworkConnection::setHandleIncomingMessageCallback(
    std::function<void(std::string const&, asio::ip::tcp::endpoint sendToEndpoint)> callback)
{
    m_handleInComingMessageCallback = callback;
}
