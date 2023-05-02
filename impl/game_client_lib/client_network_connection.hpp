#ifndef JAMTEMPLATE_CLIENT_NETWORK_CONNECTION_HPP
#define JAMTEMPLATE_CLIENT_NETWORK_CONNECTION_HPP
#include "asio/buffer.hpp"
#include <asio.hpp>
#include <array>
#include <cstdint>
#include <string>
#include <thread>

class ClientNetworkConnection {
public:
    ClientNetworkConnection(
        std::string const& ip, std::uint16_t serverPort, std::uint16_t clientPort);

    virtual ~ClientNetworkConnection();
    void establishConnection();
    void disconnect();

    void sendPing();

private:
    std::string m_ip;
    std::uint16_t m_serverPort;
    std::uint16_t m_clientPort;

    std::thread m_thread;
    std::atomic_bool m_stopThread { false };

    std::unique_ptr<asio::ip::udp::socket> m_socket { nullptr };
    asio::ip::udp::endpoint m_sendToEndpoint;

    asio::ip::udp::endpoint m_receivedFromEndpoint;
    std::array<char, 128> recv_buffer_;
    std::vector<char> m_receive_buffer;
    asio::io_context m_IOContext;

    void startReceive();
    void handleReceive(const asio::error_code& error, std::size_t /*bytes_transferred*/);
    void handle_send(std::shared_ptr<std::array<char, 1>> message, const asio::error_code& error,
        std::size_t bytes_transferred);
    void stopThread();
};

#endif // JAMTEMPLATE_CLIENT_NETWORK_CONNECTION_HPP
