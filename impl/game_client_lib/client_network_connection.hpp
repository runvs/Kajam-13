#ifndef JAMTEMPLATE_CLIENT_NETWORK_CONNECTION_HPP
#define JAMTEMPLATE_CLIENT_NETWORK_CONNECTION_HPP

#include <asio.hpp>
#include <asio/buffer.hpp>
#include <asio/executor_work_guard.hpp>
#include <compression/compressor_interface.hpp>
#include <log/logger_interface.hpp>
#include <message.hpp>
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <thread>

class ClientNetworkConnection {
public:
    ClientNetworkConnection(std::string const& ip, std::uint16_t serverPort,
        std::uint16_t clientPort, jt::LoggerInterface& logger,
        std::shared_ptr<CompressorInterface> compressor);

    virtual ~ClientNetworkConnection();

    void setHandleIncomingMessageCallback(
        std::function<void(std::string const&, asio::ip::udp::endpoint sendToEndpoint)> callback);

    void establishConnection();

    void sendInitialPing();
    void sendAlivePing(int playerId);
    void sendMessage(Message const& m);

private:
    asio::io_context m_IOContext;
    std::string m_ip;
    std::uint16_t m_serverPort;
    std::uint16_t m_clientPort;
    jt::LoggerInterface& m_logger;

    std::shared_ptr<CompressorInterface> m_compressor;

    std::thread m_thread;
    std::unique_ptr<asio::executor_work_guard<asio::io_context::executor_type>> m_workGuard;

    std::unique_ptr<asio::ip::udp::socket> m_socket { nullptr };
    asio::ip::udp::endpoint m_sendToEndpoint;

    std::function<void(std::string const&, asio::ip::udp::endpoint sendToEndpoint)>
        m_handleInComingMessageCallback;

    asio::ip::udp::endpoint m_receivedFromEndpoint;
    std::mutex m_bufferMutex;
    std::array<char, 8192> m_receiveBuffer;

    void startReceive();
    void handleReceive(const asio::error_code& error, std::size_t /*bytes_transferred*/);
    void stopThread();
    void sendString(std::string const& str);
};

#endif // JAMTEMPLATE_CLIENT_NETWORK_CONNECTION_HPP
