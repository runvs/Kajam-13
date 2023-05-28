#ifndef JAMTEMPLATE_GAME_SERVER_HPP
#define JAMTEMPLATE_GAME_SERVER_HPP

#include "asio/ip/tcp.hpp"
#include "player_info.hpp"
#include "server_network_connection.hpp"
#include <log/logger_interface.hpp>
#include <atomic>
#include <shared_mutex>
class GameServer {
public:
    GameServer(jt::LoggerInterface& logger, CompressorInterface& compressor);
    void update(float elapsed);

private:
    jt::LoggerInterface& m_logger;
    CompressorInterface& m_compressor;
    ServerNetworkConnection m_connection;

    // TODO split into player and spectators
    std::shared_mutex m_mutex;
    std::map<int, PlayerInfo> m_playerData;
    std::map<int, PlayerInfo> m_botData;

    int m_round { 1 };

    std::atomic_bool m_allPlayersReady { false };
    std::atomic_bool m_simulationStarted { false };
    std::atomic_bool m_simulationReady { false };

    int getNumberOfConnectedPlayers() const;

    // explicit copy of playerdata is desired
    void startRoundSimulation(std::map<int, PlayerInfo> const& playerData);

    // all those functions will be called from the asio thread, synchronization is needed when
    // things should be handled from the main thread
    void handleMessage(std::string const& messageContent, asio::ip::tcp::endpoint const& endpoint);
    void handleMessageInitialPing(
        std::string const& messageContent, asio::ip::tcp::endpoint const& endpoint);
    void handleMessageStayAlivePing(
        std::string const& messageContent, asio::ip::tcp::endpoint const& endpoint);
    void handleMessageRoundReady(
        std::string const& messageContent, asio::ip::tcp::endpoint const& endpoint);
    void discard(std::string const& messageContent, asio::ip::tcp::endpoint const& endpoint);
    void removePlayersIfNoAlivePingReceived(float elapsed);
    void handleMessageAddBot();
};

#endif // JAMTEMPLATE_GAME_SERVER_HPP
