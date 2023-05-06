#ifndef JAMTEMPLATE_PLAYER_INFO_HPP
#define JAMTEMPLATE_PLAYER_INFO_HPP

#include <asio.hpp>

struct PlayerInfo {
    float timeSinceLastPing { 0.0f };
    asio::ip::udp::endpoint endpoint;
};

#endif // JAMTEMPLATE_PLAYER_INFO_HPP
