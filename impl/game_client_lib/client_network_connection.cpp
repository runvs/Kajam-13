#include "client_network_connection.hpp"
#include "asio/executor_work_guard.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/udp.hpp"
#include "asio/write.hpp"
#include "game_properties.hpp"
#include <message.hpp>
#include <network_properties.hpp>
#include <nlohmann.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
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
    m_logger.debug("start thread to process async tasks", { "network", "ClientNetworkConnection" });
    m_workGuard = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(
        asio::make_work_guard(m_IOContext));
    m_thread = std::thread { [this]() { m_IOContext.run(); } };
}

void ClientNetworkConnection::handleReceive(
    const asio::error_code& /*error*/, std::size_t bytes_transferred)
{
    // Note that recv_buffer might be a long buffer, but we only use the first "bytes
    // transferred" bytes from it.
    // TODO think about adding a mutex here
    std::unique_lock<std::mutex> lock { m_bufferMutex };
    std::stringstream ss;
    ss.write(m_receiveBuffer.data(), bytes_transferred);
    auto const str = ss.str();
    lock.unlock();
    std::string uncompressed = m_compressor->decompress(str);
    std::stringstream ss_log;
    ss_log << "message received from '" << m_receivedFromEndpoint.address() << ":"
           << m_receivedFromEndpoint.port() << "'\nwith content\n"
           << uncompressed;
    m_logger.debug(ss_log.str(), { "network", "network", "ClientNetworkConnection" });
    // pass message up to be processed
    if (m_handleInComingMessageCallback) {
        m_handleInComingMessageCallback(uncompressed, m_receivedFromEndpoint);
    }
    std::unique_lock<std::mutex> lock2 { m_bufferMutex };
    m_socket->async_receive_from(asio::buffer(m_receiveBuffer), m_receivedFromEndpoint,
        std::bind(&ClientNetworkConnection::handleReceive, this, std::placeholders::_1,
            std::placeholders::_2));
    lock2.unlock();
}

void ClientNetworkConnection::sendInitialPing()
{
    m_logger.debug("send initial ping", { "network", "ClientNetworkConnection" });
    Message m;
    m.type = MessageType::InitialPing;
    sendMessage(m);
}

void ClientNetworkConnection::sendAlivePing(int playerId)
{
    m_logger.debug("send alive ping", { "network", "ClientNetworkConnection" });
    Message m;
    m.type = MessageType::StayAlivePing;
    m.playerId = playerId;
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
    asio::error_code error;
    std::string compressed = m_compressor->compress(str);
    auto size = m_socket->send_to(asio::buffer(compressed), m_sendToEndpoint, 0, error);

    // TODO think about adding a mutex here
    std::unique_lock<std::mutex> lock { m_bufferMutex };
    m_socket->async_receive_from(asio::buffer(m_receiveBuffer), m_receivedFromEndpoint,
        std::bind(&ClientNetworkConnection::handleReceive, this, std::placeholders::_1,
            std::placeholders::_2));
    lock.unlock();

    std::stringstream ss_log;
    ss_log << "message sent with size:" << size;
    ss_log << ", content: '" << str << "'";
    ss_log << ", and result: " << error.message();

    m_logger.debug(ss_log.str(), { "network", "ClientNetworkConnection" });
}

void ClientNetworkConnection::setHandleIncomingMessageCallback(
    std::function<void(std::string const&, asio::ip::udp::endpoint sendToEndpoint)> callback)
{
    m_handleInComingMessageCallback = callback;
}
