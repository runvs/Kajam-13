
#ifndef JAMTEMPLATE_SERVER_NETWORK_CONNECTION_HPP
#define JAMTEMPLATE_SERVER_NETWORK_CONNECTION_HPP

#include <asio.hpp>
#include <message.hpp>
#include <memory>
#include <set>

class ServerNetworkConnection {
public:
    ServerNetworkConnection();
    ~ServerNetworkConnection();

    void setHandleIncomingMessageCallback(
        std::function<void(std::string const&, asio::ip::udp::endpoint sendToEndpoint)> callback);

    void handleReceive(const asio::error_code& error, std::size_t /*bytes_transferred*/);
    void update();
    void sendMessage(Message const& m, asio::ip::udp::endpoint sendToEndpoint);

private:
    asio::io_context m_IOContext;
    std::unique_ptr<asio::ip::udp::socket> m_socket { nullptr };
    asio::ip::udp::endpoint m_remote_endpoint;

    std::thread m_thread;
    std::unique_ptr<asio::executor_work_guard<asio::io_context::executor_type>> m_workGuard;

    std::array<char, 1024> m_receiveBuffer;

    std::function<void(std::string const&, asio::ip::udp::endpoint sendToEndpoint)>
        m_handleInComingMessageCallback;

    void handleMessage(std::string const& str, asio::ip::udp::endpoint endpoint);
    void sendStringTo(std::string const& str, asio::ip::udp::endpoint sendToEndpoint);

    void awaitNextMessageInternal();
};

#endif // JAMTEMPLATE_SERVER_NETWORK_CONNECTION_HPP
