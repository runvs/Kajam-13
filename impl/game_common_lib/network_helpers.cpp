#include "network_helpers.hpp"
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <performance_measurement.hpp>
#include <iomanip>
#include <sstream>

void NetworkHelpers::freeSendString(
    std::string const& str, asio::ip::tcp::socket& socket, jt::LoggerInterface& logger)
{
    auto const start = std::chrono::system_clock::now();

    if (!socket.is_open()) {
        logger.error("socket closed", { "NetworkHelpers", "freeSendString" });
    }
    asio::error_code error;

    // write size information of string with fixed length
    std::stringstream buffer;
    buffer << std::setfill(' ') << std::setw(32) << str.size() << str;
    logger.verbose("sending buffer: " + buffer.str());
    // Note: keep payload alive
    auto payload_ptr = std::make_shared<std::string>(buffer.str());
    asio::async_write(socket, asio::buffer(*payload_ptr, payload_ptr->size()),
        [payload_ptr, &logger, &socket, start](auto const& error, auto size) {
            auto const duration_of_send = jt::getDurationInSecondsSince(start);
            if (error) {
                logger.error(error.message(), { "network", "freeSendString" });
                socket.close();
            } else {
                logger.debug("message sent with size: " + std::to_string(size) + ", took "
                        + std::to_string(duration_of_send) + "s",
                    { "network", "freeSendString" });
            }
        });
}

void NetworkHelpers::freeHandleReceive(const asio::error_code& error, std::size_t bytes_transferred,
    asio::ip::tcp::socket& socket, ReceiveBuffer& buffer, jt::LoggerInterface& logger,
    std::function<void(const std::string&)> handlerFunction)
{
    if (error) {
        std::stringstream ss;
        ss << error << " : " << error.message() << " : " << bytes_transferred << std::endl;
        logger.error(ss.str(), { "network", "error" });
        socket.close();
        return;
    }
    if (!socket.is_open()) {
        logger.error("socket closed", { "NetworkHelpers", "freeHandleReceive" });
    }

    auto const bytesToRead = std::stoul(std::string(buffer.size.cbegin(), buffer.size.cend()));
    logger.debug("handle receive with bytes: " + std::to_string(bytesToRead),
        { "network", "ClientNetworkConnection" });
    if (bytesToRead > buffer.data.size()) {
        throw std::invalid_argument { "message too big." };
    }
    asio::error_code ec;
    auto const bytesRead = asio::read(socket, asio::buffer(buffer.data.data(), bytesToRead), ec);
    if (bytesRead != bytesToRead) {
        logger.fatal("bytes to read and bytes read do not match");
        socket.close();
        return;
    }
    if (ec) {
        logger.error("read failed with message " + ec.message());
        socket.close();
        return;
    }

    // Note that recv_buffer might be a long buffer, but we only use the first "bytesToRead" bytes
    // from it.
    std::stringstream ss;
    ss.write(buffer.data.data(), bytesToRead);
    auto const str = ss.str();
    handlerFunction(str);
}

bool NetworkHelpers::endpointsMatch(
    asio::ip::tcp::endpoint const& ep1, asio::ip::tcp::endpoint const& ep2)
{
    return ep1.address() == ep2.address() && ep1.port() == ep2.port();
}
