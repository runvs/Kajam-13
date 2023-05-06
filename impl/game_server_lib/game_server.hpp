#ifndef JAMTEMPLATE_GAME_SERVER_HPP
#define JAMTEMPLATE_GAME_SERVER_HPP

#include "player_info.hpp"
#include "server_network_connection.hpp"
class GameServer {
public:
    GameServer();
    void update(float elapsed);

private:
    ServerNetworkConnection m_connection;
    // TODO split into player and spectators
    // TODO make second type in map a PlayerInfoStruct
    std::map<int, PlayerInfo> m_playerData;
    float m_receiveTimer { 0.0f };

    int m_round { 1 };
    int m_playersReady { 0 };
    int m_connectedPlayers { 0 };
};

#endif // JAMTEMPLATE_GAME_SERVER_HPP
