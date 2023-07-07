#ifndef JAMTEMPLATE_GAME_SERVER_HPP
#define JAMTEMPLATE_GAME_SERVER_HPP

#include <asio/ip/tcp.hpp>
#include <game_simulation.hpp>
#include <log/logger_interface.hpp>
#include <network_data/select_unit_info.hpp>
#include <network_data/unit_info_collection.hpp>
#include <player_info.hpp>
#include <server_network_connection.hpp>
#include <upgrade_unit_data.hpp>
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>

class GameServer {
public:
    GameServer(jt::LoggerInterface& logger, CompressorInterface& compressor);

    void update(float elapsed);

private:
    jt::LoggerInterface& m_logger;
    CompressorInterface& m_compressor;
    ServerNetworkConnection m_connection;

    std::mutex m_mutex;

    std::map<int, PlayerInfo> m_playerData;
    std::map<int, PlayerInfo> m_botData;
    bool m_matchHasStarted { false };

    int m_round { 1 };

    std::atomic_bool m_allPlayersReady { false };
    std::atomic_bool m_simulationStarted { false };

    UnitInfoCollection m_unitInfos;
    std::unique_ptr<SelectUnitInfoCollection> m_startingUnits { nullptr };
    std::unique_ptr<GameSimulation> m_gameSimulation { nullptr };

    int getNumberOfConnectedPlayers() const;
    void checkForAllPlayersConnected();

    // explicit copy of player data is desired
    void startRoundSimulation();

    // all those functions will be called from the asio thread, synchronization is needed when
    // things should be handled from the main thread
    void handleMessage(std::string const& messageContent, asio::ip::tcp::endpoint const& endpoint);
    void handleMessageInitialPing(
        std::string const& messageContent, asio::ip::tcp::endpoint const& endpoint);
    void handleMessageRoundReady(
        std::string const& messageContent, asio::ip::tcp::endpoint const& endpoint);
    void handleMessageUnitUpgrade(Message const& m);
    void handleMessageAddBot();

    void discard(std::string const& messageContent, asio::ip::tcp::endpoint const& endpoint);

    void removePlayerIfConnectionClosed(float elapsed);
    void performAI(std::map<int, PlayerInfo>& botDataCopy) const;

    void resetServer();
};

#endif // JAMTEMPLATE_GAME_SERVER_HPP
