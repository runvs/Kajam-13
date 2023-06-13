#ifndef JAMTEMPLATE_PLAYER_INFO_HPP
#define JAMTEMPLATE_PLAYER_INFO_HPP

#include "client_placement_data.hpp"
#include <asio.hpp>

struct PlayerInfo {
    float timeSinceLastPing { 0.0f };
    asio::ip::tcp::endpoint endpoint;
    bool roundReady { false };
    ClientPlacementData roundEndPlacementData;
};

#endif // JAMTEMPLATE_PLAYER_INFO_HPP
