#ifndef JAMTEMPLATE_NETWORK_HELPERS_HPP
#define JAMTEMPLATE_NETWORK_HELPERS_HPP

#include <asio.hpp>
#include <log/logger_interface.hpp>
#include <iostream>
#include <sstream>
#include <string>

namespace NetworkHelpers {

// TODO pass logger instead of cout
void freeSendString(
    std::string const& str, asio::ip::tcp::socket& socket, jt::LoggerInterface& logger);

// TODO group sizeBuffer and DataBuffer in a class
void freeHandleReceive(asio::error_code const& error, std::size_t bytes_transferred,
    asio::ip::tcp::socket& socket, std::array<char, 32>& sizeBuffer,
    std::array<char, 102400>& dataBuffer, jt::LoggerInterface& logger,
    std::function<void(std::string const&)> handlerFunction);

}; // namespace NetworkHelpers

#endif // JAMTEMPLATE_NETWORK_HELPERS_HPP
