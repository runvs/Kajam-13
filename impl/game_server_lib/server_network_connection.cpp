#include "server_network_connection.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/write.hpp"
#include "compression/compressor_interface.hpp"
#include "message.hpp"
#include <network_properties.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>

ServerNetworkConnection::ServerNetworkConnection(CompressorInterface& compressor)
    : m_compressor { compressor }
    , m_acceptor { m_IOContext,
        asio::ip::tcp::endpoint {
            NetworkProperties::NetworkProtocolType(), NetworkProperties::DefaultServerPort() } }
{
    // TODO pass in logger and use logger instead of cout
    std::cout << "start thread to handle async tasks\n";
    m_socket = std::make_unique<asio::ip::tcp::socket>(m_IOContext);
    // todo make the socket accept more than one connectionn
    m_acceptor.accept(*m_socket);
    std::cout << "connection accepted\n";
    awaitNextMessageInternal();

    m_workGuard = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(
        asio::make_work_guard(m_IOContext));
    m_thread = std::thread { [this]() {
        while (true) {
            m_IOContext.run();
        }
    } };
}

ServerNetworkConnection::~ServerNetworkConnection()
{
    m_IOContext.stop();
    m_socket->close();
    m_thread.join();

    m_socket.reset();
}

void ServerNetworkConnection::setHandleIncomingMessageCallback(
    std::function<void(std::string const&, asio::ip::tcp::endpoint sendToEndpoint)> callback)
{
    m_handleInComingMessageCallback = callback;
}

void ServerNetworkConnection::update() { }

void ServerNetworkConnection::handleReceive(const asio::error_code& error, std::size_t length)
{
    std::lock_guard<std::mutex> lock { m_mutex };
    if (error) {
        // TODO use logger
        std::cerr << error << " : " << error.message() << " : " << length << std::endl;
        m_socket->close();
        return;
    }

    std::stringstream ss;
    ss.write(m_receiveBuffer.data(), length);
    auto const str = ss.str();
    std::string uncompressed = m_compressor.decompress(str);
    handleMessage(uncompressed, m_remote_endpoint);
    awaitNextMessageInternal();
}

void ServerNetworkConnection::awaitNextMessageInternal()
{
    // TODO receive size first, then parse the actual data
    m_socket->async_receive(
        asio::buffer(m_receiveBuffer), [this](auto ec, auto len) { handleReceive(ec, len); });
}

void ServerNetworkConnection::handleMessage(
    std::string const& str, asio::ip::tcp::endpoint endpoint)
{
    std::cout << "received message from endpoint: " << endpoint.address() << ":" << endpoint.port()
              << "'\n";
    // generic message handling
    if (m_handleInComingMessageCallback) {
        m_handleInComingMessageCallback(str, endpoint);
    }
}

void ServerNetworkConnection::sendMessage(const Message& m, asio::ip::tcp::endpoint sendToEndpoint)
{
    if (!m_socket || !m_socket->is_open()) {
        std::cerr << "socket not open\n";
        return;
    }

    nlohmann::json j = m;
    sendStringTo(j.dump(), sendToEndpoint);
}

void ServerNetworkConnection::sendStringTo(
    const std::string& str, asio::ip::tcp::endpoint /*sendToEndpoint*/)
{
    asio::error_code error;
    std::string const compressed = m_compressor.compress(str);
    //    auto size = m_socket->send(asio::buffer(compressed), 0, error);
    std::string header;
    header.resize(32 + 1);
    std::sprintf(header.data(), "%32lu", static_cast<std::size_t>(compressed.size()));
    auto const s1 = asio::write(*m_socket, asio::buffer(header, 32), error);
    if (s1 != 32) {
        std::cerr << "not written header correctly\n";
    } else {
        std::cout << "written header: '" << header << "'\n";
    }
    auto size = asio::write(*m_socket, asio::buffer(compressed), error);
    if (error) {
        std::cout << error.message() << std::endl;
        m_socket->close();
    }
    std::cout << "message sent with size: " << size << std::endl;
}
