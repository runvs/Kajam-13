
#include "network_helpers.hpp"
void NetworkHelpers::freeSendString(
     std::string const& str, asio::ip::tcp::socket& socket, jt::LoggerInterface& logger)
{
    asio::error_code error;

    // write size information of string with fixed length
    std::string header;
    header.resize(32 + 1);
    std::sprintf(header.data(), "%32lu", static_cast<unsigned long>(str.size()));
    // TODO should be async
    asio::write(socket, asio::buffer(header, 32), error);
    // TODO check error value

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

    auto const bytesToRead = std::stoul(std::string(buffer.size.cbegin(), buffer.size.cend()));
    logger.debug("handle receive with bytes: " + std::to_string(bytesToRead),
        { "network", "ClientNetworkConnection" });
    if (bytesToRead > buffer.data.size()) {
        throw std::invalid_argument { "message too big." };
    }
    // TODO could be async
    asio::read(socket, asio::buffer(buffer.data.data(), bytesToRead));

    // Note that recv_buffer might be a long buffer, but we only use the first "bytesToRead" bytes
    // from it.
    std::stringstream ss;
    ss.write(buffer.data.data(), bytesToRead);
    auto const str = ss.str();
    handlerFunction(str);
}