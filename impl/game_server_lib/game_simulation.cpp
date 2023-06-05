#include "game_simulation.hpp"
#include "unit_info.hpp"
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

void GameSimulation::prepareSimulationForNewRound()
{
    m_simulationObjects.clear();
    for (auto const& props : m_unitInformationForRoundStart) {
        // TODO get unit info for specific type instead of generic unit info without info
        UnitInfo unitInfo;
        unitInfo.movementSpeed = 1.0f;
        auto obj = std::make_unique<ServerUnit>(unitInfo);
        obj->updateState(props);
        m_simulationObjects.emplace_back(std::move(obj));
    }
}

void GameSimulation::addUnit(const ObjectProperties& props)
{
    if (!checkIfUnitIsUnique(props)) {
        return;
    }
    m_unitInformationForRoundStart.push_back(props);
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

    m_logger.info("simulation finished");
    sender.sendSimulationResults(propertiesForAllUnitsForAllRounds);
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
bool GameSimulation::checkIfUnitIsUnique(const ObjectProperties& newUnitProps)
{
    for (auto const& props : m_unitInformationForRoundStart) {
        if (props.ints.at(jk::unitID) == newUnitProps.ints.at(jk::unitID)
            && props.ints.at(jk::playerID) == newUnitProps.ints.at(jk::playerID)) {
            m_logger.warning("Adding a unit that is already present in the game simulation");
            return false;
        }
    }
    return true;
}
