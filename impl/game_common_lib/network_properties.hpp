#ifndef JAMTEMPLATE_NETWORK_PROPERTIES_HPP
#define JAMTEMPLATE_NETWORK_PROPERTIES_HPP

#include <asio.hpp>
#include <cstdint>
#include <functional>

class NetworkProperties {
public:
    static auto NetworkProtocolType() { return asio::ip::udp::v6(); }

    static std::uint16_t DefaultServerPort() { return 1500; }
    static std::uint16_t DefaultClientPort() { return 1501; }

    static float AlivePingTimer() { return 1.0f; }
};

#endif // JAMTEMPLATE_NETWORK_PROPERTIES_HPP
