#include "game_simulation.hpp"
#include "object_properties.hpp"
#include "vector.hpp"
#include <game_properties.hpp>
#include <server_unit.hpp>
#include <memory>
GameSimulation::GameSimulation(jt::LoggerInterface& logger)
    : m_logger { logger }
{
}

void GameSimulation::updateSimulationForNewRound(std::map<int, PlayerInfo> const& playerData)
{
    m_latestPlayerData = playerData;
    m_simulationObjects.clear();
    for (auto const& kvp : m_latestPlayerData) {
        for (auto const& props : kvp.second.roundEndPlacementData.m_properties) {
            auto obj = std::make_unique<ServerUnit>();
            jt::Vector2f const pos { props.floats.at("x"), props.floats.at("y") };
            obj->setUnitID(props.ints.at("i"));
            obj->setPosition(pos);
            m_simulationObjects.emplace_back(std::move(obj));
        }
    }
}

void GameSimulation::performSimulation(SimulationResultMessageSender& sender)
{
    auto const timePerUpdate = 0.005f;
    std::vector<std::vector<ObjectProperties>> propertiesForAllUnitsForAllRounds;
    for (auto i = 0u; i != GP::NumberOfStepsPerRound(); ++i) {
        std::vector<ObjectProperties> propertiesForAllUnitsForOneRound;
        for (auto& obj : m_simulationObjects) {
            obj->update(timePerUpdate);

            auto data = obj->saveState();
            data.ints["simulationTick"] = i;
            propertiesForAllUnitsForOneRound.push_back(data);
        }
        propertiesForAllUnitsForAllRounds.push_back(propertiesForAllUnitsForOneRound);
    }
    std::vector<asio::ip::tcp::endpoint> endpoints;
    for (auto const& kvp : m_latestPlayerData) {
        endpoints.push_back(kvp.second.endpoint);
    }
    m_logger.info("simulation finished");
    sender.sendSimulationResults(propertiesForAllUnitsForAllRounds, endpoints);
    m_logger.info("sending of simulation results done");
}
