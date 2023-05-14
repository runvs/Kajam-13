
#ifndef JAMTEMPLATE_GAME_SIMULATION_HPP
#define JAMTEMPLATE_GAME_SIMULATION_HPP

#include "simulation_result_message_sender.hpp"
#include <log/logger_interface.hpp>
#include <player_info.hpp>
#include <server_network_connection.hpp>
#include <server_unit.hpp>
#include <simulation_object_interface.hpp>
#include <map>
#include <mutex>

class GameSimulation {
public:
    explicit GameSimulation(jt::LoggerInterface& logger);
    void updateSimulationForNewRound(std::map<int, PlayerInfo> const& playerData);
    void performSimulation(SimulationResultMessageSender& sender);

private:
    jt::LoggerInterface& m_logger;
    std::map<int, PlayerInfo> m_latestPlayerData;

    std::vector<std::unique_ptr<SimulationObjectInterface>> m_simulationObjects;
};

#endif // JAMTEMPLATE_GAME_SIMULATION_HPP
