#ifndef JAMTEMPLATE_NETWORK_HELPERS_HPP
#define JAMTEMPLATE_NETWORK_HELPERS_HPP

#include <asio/error_code.hpp>
#include <asio/ip/tcp.hpp>
#include <log/logger_interface.hpp>
#include <receive_buffer.hpp>
#include <cstddef>
#include <functional>
#include <string>

namespace NetworkHelpers {

void freeSendString(
    std::string const& str, asio::ip::tcp::socket& socket, jt::LoggerInterface& logger);

void freeHandleReceive(asio::error_code const& error, std::size_t bytes_transferred,
    asio::ip::tcp::socket& socket, ReceiveBuffer& buffer, jt::LoggerInterface& logger,
    std::function<void(std::string const&)> handlerFunction);

}; // namespace NetworkHelpers

#endif // JAMTEMPLATE_NETWORK_HELPERS_HPP
