#ifndef JAMTEMPLATE_GAME_SERVER_HPP
#define JAMTEMPLATE_GAME_SERVER_HPP

#include "asio/ip/udp.hpp"
#include "player_info.hpp"
#include "server_network_connection.hpp"
#include <log/logger_interface.hpp>
class GameServer {
public:
    GameServer(jt::LoggerInterface& logger);
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

    jt::LoggerInterface& m_logger;

    void handleMessage(std::string const& messageContent, asio::ip::udp::endpoint const& endpoint);
    void handleMessageInitialPing(
        std::string const& messageContent, asio::ip::udp::endpoint const& endpoint);
    void handleMessageStayAlivePing(
        std::string const& messageContent, asio::ip::udp::endpoint const& endpoint);
    void handleMessageRoundReady(
        std::string const& messageContent, asio::ip::udp::endpoint const& endpoint);
    void discard(std::string const& messageContent, asio::ip::udp::endpoint const& endpoint);
};

#endif // JAMTEMPLATE_GAME_SERVER_HPP
