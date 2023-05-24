#include "game_simulation.hpp"
#include <game_properties.hpp>
#include <json_keys.hpp>
#include <math_helper.hpp>
#include <object_properties.hpp>
#include <server_unit.hpp>
#include <vector.hpp>
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
        for (auto& props : kvp.second.roundEndPlacementData.m_properties) {
            auto obj = std::make_unique<ServerUnit>();
            obj->updateState(props);
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
            obj->update(timePerUpdate, *this);

            auto data = obj->saveState();
            data.ints[jk::simulationTick] = i;

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

jt::Vector2f GameSimulation::getClosestTargetTo(const jt::Vector2f& pos, int playerId)
{
    // TODO using a SpatialObjectGrid might be useful here?

    float minDistance = 999999999.9f;
    jt::Vector2f targetPos { 0.0f, 0.0f };
    for (auto const& obj : m_simulationObjects) {
        if (obj->getPlayerID() == playerId) {
            continue;
        }

        auto const& otherp = obj->getPosition();
        auto const dist = otherp - pos;
        auto l2 = jt::MathHelper::lengthSquared(dist);
        if (l2 < minDistance) {
            minDistance = l2;
            targetPos = otherp;
        }
    }
    return targetPos;
}
