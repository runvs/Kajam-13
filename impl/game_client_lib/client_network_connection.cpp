#include "client_network_connection.hpp"
#include "asio/executor_work_guard.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/write.hpp"
#include "game_properties.hpp"
#include <message.hpp>
#include <network_properties.hpp>
#include <nlohmann.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

ClientNetworkConnection::ClientNetworkConnection(std::string const &ip, std::uint16_t serverPort,
                                                 std::uint16_t clientPort, jt::LoggerInterface &logger,
                                                 std::shared_ptr<CompressorInterface> compressor)
        : m_ip{ip}, m_serverPort{serverPort}, m_clientPort{clientPort}, m_logger{logger}, m_compressor{compressor} {
}

ClientNetworkConnection::~ClientNetworkConnection() {
    m_logger.debug(
            "ClientNetworkConnection destructor called", {"network", "ClientNetworkConnection"});
    m_logger.verbose("close socket", {"network", "ClientNetworkConnection"});
    m_socket->close();
    m_logger.verbose("stop thread", {"stop thread", "ClientNetworkConnection"});
    stopThread();
    m_logger.verbose("reset socket pointer", {"network", "ClientNetworkConnection"});
    m_socket.reset();
}

void ClientNetworkConnection::establishConnection() {
    asio::ip::tcp::resolver resolver(m_IOContext);
    m_socket = std::make_unique<asio::ip::tcp::socket>(m_IOContext,
                                                       asio::ip::tcp::endpoint{NetworkProperties::NetworkProtocolType(),
                                                                               m_clientPort});
    auto endpoints = resolver.resolve(
            NetworkProperties::NetworkProtocolType(), m_ip, std::to_string(m_serverPort));
    m_sendToEndpoint = asio::connect(*m_socket, endpoints);

    startProcessing();
}

void ClientNetworkConnection::startProcessing() {
    m_logger.debug("start thread to process async tasks", {"network", "ClientNetworkConnection"});
    m_workGuard = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(
            asio::make_work_guard(m_IOContext));
    m_thread = std::thread{[this]() { m_IOContext.run(); }};
}

void ClientNetworkConnection::handleReceive(
        asio::error_code const &error, std::size_t bytes_transferred) {
    if (error) {
        std::stringstream ss;
        ss << error << " : " << error.message() << " : " << bytes_transferred << std::endl;
        m_logger.error(ss.str(), {"network", "error"});
        m_socket->close();
        return;
    }

    auto const bytesToRead = std::stoul(std::string(m_sizeBuffer.begin(), m_sizeBuffer.end()));
    m_logger.info("handle receive with bytes: " + std::to_string(bytesToRead),
                  {"network", "ClientNetworkConnection"});
    if (bytesToRead > m_receiveBuffer.size()) {
        throw std::invalid_argument{"message too big."};
    }
    asio::read(*m_socket, asio::buffer(m_receiveBuffer.data(), bytesToRead));

    // Note that recv_buffer might be a long buffer, but we only use the first "bytesToRead" bytes from it.
    std::stringstream ss;
    ss.write(m_receiveBuffer.data(), bytesToRead);
    auto const str = ss.str();

    std::string const uncompressed = m_compressor->decompress(str);

    std::stringstream ss_log;
    ss_log << "message received from '" << m_receivedFromEndpoint.address() << ":"
           << m_receivedFromEndpoint.port() << "'\nwith content\n"
           << uncompressed;
    m_logger.debug(ss_log.str(), {"network", "ClientNetworkConnection"});

    // pass message up to be processed
    if (m_handleInComingMessageCallback) {
        m_handleInComingMessageCallback(uncompressed, m_receivedFromEndpoint);
    }

    // queue next handler so the next message will be processed as well
    awaitNextMessage();
}

void ClientNetworkConnection::awaitNextMessage() {
    asio::async_read(*m_socket, asio::buffer(m_sizeBuffer.data(), m_sizeBuffer.size()),
                     [this](auto ec, auto len) {
                         std::unique_lock<std::mutex> lock{m_bufferMutex};
                         handleReceive(ec, len);
                     });
}

void ClientNetworkConnection::sendInitialPing() {
    m_logger.debug("send initial ping", {"network", "ClientNetworkConnection"});
    Message m;
    m.type = MessageType::InitialPing;
    sendMessage(m);
}

void ClientNetworkConnection::sendAlivePing(int playerId) {
    m_logger.debug("send alive ping", {"network", "ClientNetworkConnection"});
    Message m;
    m.type = MessageType::StayAlivePing;
    m.playerId = playerId;
    sendMessage(m);
}

void ClientNetworkConnection::stopThread() {
    m_workGuard.reset();
    m_IOContext.stop();
    m_thread.join();
}

void ClientNetworkConnection::sendMessage(const Message &m) {
    if (!m_socket) {
        m_logger.warning(
                "send message called with nullptr socket", {"network", "ClientNetworkConnection"});
        return;
    }

    nlohmann::json j = m;
    sendString(j.dump());
}

void ClientNetworkConnection::sendString(const std::string &str) {
    asio::error_code error;
    std::string compressed = m_compressor->compress(str);
    std::unique_lock<std::mutex> lock{m_bufferMutex};
    // TODO write size in front of every message
    asio::write(*m_socket, asio::buffer(compressed));
    lock.unlock();

    if (!m_alreadyReceiving.exchange(true)) {
        awaitNextMessage();
    }

    std::stringstream ss_log;
    ss_log << "message sent with content: '" << str << "'";
    ss_log << ", and result: " << error.message();

    m_logger.debug(ss_log.str(), {"network", "ClientNetworkConnection"});
}

void ClientNetworkConnection::setHandleIncomingMessageCallback(
        std::function<void(std::string const &, asio::ip::tcp::endpoint sendToEndpoint)> callback) {
    m_handleInComingMessageCallback = callback;
}
