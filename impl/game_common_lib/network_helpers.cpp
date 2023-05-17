
#include "network_helpers.hpp"
void NetworkHelpers::freeSendString(
    const std::string& str, asio::ip::tcp::socket& socket, jt::LoggerInterface& logger)
{
    asio::error_code error;

    // write size information of string with fixed length
    std::string header;
    header.resize(32 + 1);
    std::sprintf(header.data(), "%32lu", str.size());
    asio::write(socket, asio::buffer(header, 32), error);

    // write actual string with dynamic length
    auto const size = asio::write(socket, asio::buffer(str), error);
    if (error) {
        logger.error(error.message(), { "network", "freeSendString" });
        socket.close();
    } else {
        logger.debug(
            "message sent with size: " + std::to_string(size), { "network", "freeSendString" });
    }
}

void NetworkHelpers::freeHandleReceive(const asio::error_code& error, std::size_t bytes_transferred,
    asio::ip::tcp::socket& socket, std::array<char, 32>& sizeBuffer,
    std::array<char, 102400>& dataBuffer, jt::LoggerInterface& logger,
    std::function<void(const std::string&)> handlerFunction)
{
    if (error) {
        std::stringstream ss;
        ss << error << " : " << error.message() << " : " << bytes_transferred << std::endl;
        logger.error(ss.str(), { "network", "error" });
        socket.close();
        return;
    }

    auto const bytesToRead = std::stoul(std::string(sizeBuffer.cbegin(), sizeBuffer.cend()));
    logger.debug("handle receive with bytes: " + std::to_string(bytesToRead),
        { "network", "ClientNetworkConnection" });
    if (bytesToRead > dataBuffer.size()) {
        throw std::invalid_argument { "message too big." };
    }
    asio::read(socket, asio::buffer(dataBuffer.data(), bytesToRead));

    // Note that recv_buffer might be a long buffer, but we only use the first "bytesToRead" bytes
    // from it.
    std::stringstream ss;
    ss.write(dataBuffer.data(), bytesToRead);
    auto const str = ss.str();
    handlerFunction(str);
}
