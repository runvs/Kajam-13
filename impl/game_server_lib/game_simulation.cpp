#include "game_simulation.hpp"
#include <game_properties.hpp>
#include <server_unit.hpp>
GameSimulation::GameSimulation(jt::LoggerInterface& logger)
    : m_logger { logger }
{
}

void GameSimulation::updateSimulationForNewRound(std::map<int, PlayerInfo> const& playerData)
{
    m_latestPlayerData = playerData;
    // TODO update local round setup
}

void GameSimulation::performSimulation(ServerNetworkConnection& connection)
{
    // TODO store this locally
    ServerUnit su;
    su.setPosition(m_latestPlayerData.at(0).roundEndPlacementData.m_position);

    for (auto i = 0u; i != GP::NumberOfStepsPerRound(); ++i) {
        auto const timePerUpdate = 0.005f;
        su.update(timePerUpdate);
        Message m;
        m.type = MessageType::SimulationResult;
        auto data = su.saveState();
        data.ints["simulationTick"] = i;
        nlohmann::json j = data;
        m.data = j.dump();
        // TODO send to all connected players
        connection.sendMessage(m, m_latestPlayerData.at(0).endpoint);
        // TODO sleep seems to be necessary otherwise client will get hickups (because of race
        // condition?)
        std::this_thread::sleep_for(std::chrono::milliseconds { 5 });
    }

    m_logger.info("simulation finished");
}
