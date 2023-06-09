#ifndef JAMTEMPLATE_NETWORK_PROPERTIES_HPP
#define JAMTEMPLATE_NETWORK_PROPERTIES_HPP

#include <asio/ip/tcp.hpp>
#include <cstdint>
#include <string>

class NetworkProperties {
public:
    static auto NetworkProtocolType() { return asio::ip::tcp::v4(); }
    static std::string DefaultIPAddress() { return "127.0.0.1"; }

    static std::uint16_t DefaultServerPort() { return 1500; }
    static std::uint16_t DefaultClientPort() { return 1501; }

    static float floatPrecision() { return 10.0f; }
};

#endif // JAMTEMPLATE_NETWORK_PROPERTIES_HPP
