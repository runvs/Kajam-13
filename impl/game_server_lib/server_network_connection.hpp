
#ifndef JAMTEMPLATE_SERVER_NETWORK_CONNECTION_HPP
#define JAMTEMPLATE_SERVER_NETWORK_CONNECTION_HPP

#include "log/logger_interface.hpp"
#include <asio.hpp>
#include <compression/compressor_interface.hpp>
#include <message.hpp>
#include <memory>
#include <set>

class ServerNetworkConnection {
public:
    ServerNetworkConnection(CompressorInterface& compressor, jt::LoggerInterface& logger);
    ~ServerNetworkConnection();

    void startProcessing();

    void setHandleIncomingMessageCallback(
        std::function<void(std::string const&, asio::ip::tcp::endpoint sendToEndpoint)> callback);

    void handleReceive(const asio::error_code& error, std::size_t /*bytes_transferred*/);
    void update();
    void sendMessage(Message const& m, asio::ip::tcp::endpoint sendToEndpoint);

private:
    CompressorInterface& m_compressor;
    jt::LoggerInterface& m_logger;
    asio::io_context m_IOContext;
    std::unique_ptr<asio::ip::tcp::socket> m_socket { nullptr };
    asio::ip::tcp::endpoint m_remote_endpoint;
    asio::ip::tcp::acceptor m_acceptor;

    std::thread m_thread;
    std::unique_ptr<asio::executor_work_guard<asio::io_context::executor_type>> m_workGuard;

    std::mutex m_bufferMutex;
    std::array<char, 32> m_sizeBuffer;
    std::array<char, 102400> m_receiveBuffer;

    std::function<void(std::string const&, asio::ip::tcp::endpoint sendToEndpoint)>
        m_handleInComingMessageCallback;

    void handleMessage(std::string const& str, asio::ip::tcp::endpoint endpoint);
    void sendStringTo(std::string const& str, asio::ip::tcp::endpoint sendToEndpoint);

    void awaitNextMessageInternal();
};

#endif // JAMTEMPLATE_SERVER_NETWORK_CONNECTION_HPP
