
#ifndef JAMTEMPLATE_GAME_SIMULATION_HPP
#define JAMTEMPLATE_GAME_SIMULATION_HPP

#include "server_network_connection.hpp"
#include <log/logger_interface.hpp>
#include <player_info.hpp>
#include <map>

class GameSimulation {
public:
    explicit GameSimulation(jt::LoggerInterface& logger);
    void updateSimulationForNewRound(std::map<int, PlayerInfo> const& playerData);
    void performSimulation(ServerNetworkConnection& connection);

private:
    jt::LoggerInterface& m_logger;
    std::map<int, PlayerInfo> m_latestPlayerData;
};

#endif // JAMTEMPLATE_GAME_SIMULATION_HPP
