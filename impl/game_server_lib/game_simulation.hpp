
#ifndef JAMTEMPLATE_GAME_SIMULATION_HPP
#define JAMTEMPLATE_GAME_SIMULATION_HPP

#include "simulation_result_message_sender.hpp"
#include <log/logger_interface.hpp>
#include <player_info.hpp>
#include <server_network_connection.hpp>
#include <server_unit.hpp>
#include <simulation_object_interface.hpp>
#include <world_info_interface.hpp>
#include <map>
#include <mutex>

class GameSimulation : public WorldInfoInterface {
public:
    explicit GameSimulation(jt::LoggerInterface& logger);
    void updateSimulationForNewRound(std::map<int, PlayerInfo> const& playerData);
    void performSimulation(SimulationResultMessageSender& sender);
    // TODO think about using an optional here, as there might not be a valid target.
    // TODO otherwise, just return the passed in position?
    jt::Vector2f getClosestTargetTo(const jt::Vector2f& pos, int playerId) override;

private:
    jt::LoggerInterface& m_logger;
    std::map<int, PlayerInfo> m_latestPlayerData;

    std::vector<std::unique_ptr<SimulationObjectInterface>> m_simulationObjects;
};

#endif // JAMTEMPLATE_GAME_SIMULATION_HPP
