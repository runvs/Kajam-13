
#ifndef JAMTEMPLATE_SERVER_NETWORK_CONNECTION_HPP
#define JAMTEMPLATE_SERVER_NETWORK_CONNECTION_HPP

#include "log/logger_interface.hpp"
#include "receive_buffer.hpp"
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

    void handleReceive(asio::ip::tcp::socket& socket, const asio::error_code& error, std::size_t);
    void update();

    void sendMessageToAll(Message const& m);

    void sendMessageToOne(Message const& m, asio::ip::tcp::endpoint const& endpoint);

private:
    CompressorInterface& m_compressor;
    jt::LoggerInterface& m_logger;
    asio::io_context m_IOContext;

    std::mutex m_socketsMutex;
    std::vector<std::unique_ptr<asio::ip::tcp::socket>> m_sockets;

    asio::ip::tcp::acceptor m_acceptor;

    std::thread m_thread;

    ReceiveBuffer m_buffer;

    std::function<void(std::string const&, asio::ip::tcp::endpoint sendToEndpoint)>
        m_handleInComingMessageCallback;

    void handleMessage(std::string const& str, asio::ip::tcp::endpoint endpoint);
    void sendStringTo(std::string const& str, asio::ip::tcp::socket& socket);

    void awaitNextMessageInternal(asio::ip::tcp::socket& socket);

    void handleAccept(asio::error_code ec, asio::ip::tcp::socket&& socket);
    void awaitNextAccept();
};

#endif // JAMTEMPLATE_SERVER_NETWORK_CONNECTION_HPP
