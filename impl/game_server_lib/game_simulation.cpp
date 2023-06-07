#include "game_simulation.hpp"
#include <game_properties.hpp>
#include <json_keys.hpp>
#include <math_helper.hpp>
#include <object_properties.hpp>
#include <server_unit.hpp>
#include <simulation_object_interface.hpp>
#include <unit_info.hpp>
#include <vector.hpp>
#include <memory>

GameSimulation::GameSimulation(jt::LoggerInterface& logger, UnitInfoCollection& unitInfos)
    : m_logger { logger }
    , m_unitInfos { unitInfos }
    , m_world { std::make_shared<Terrain>() } // TODO: use terrain for calculations
{
}

void GameSimulation::prepareSimulationForNewRound()
{
    m_simulationObjects.clear();
    for (auto const& props : m_unitInformationForRoundStart) {
        auto obj = std::make_shared<ServerUnit>(
            m_unitInfos.getInfoForType(props.strings.at(jk::unitType)));
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

std::weak_ptr<SimulationObjectInterface> GameSimulation::getClosestTargetTo(
    const jt::Vector2f& pos, int playerId)
{
    // TODO using a SpatialObjectGrid might be useful here?

    float minDistance = 999999999.9f;
    std::shared_ptr<SimulationObjectInterface> target { nullptr };
    for (auto const& obj : m_simulationObjects) {
        if (!obj->isAlive()) {
            continue;
        }
        if (obj->getPlayerID() == playerId) {
            continue;
        }

        auto const& otherp = obj->getPosition();
        auto const dist = otherp - pos;
        auto l2 = jt::MathHelper::lengthSquared(dist);
        if (l2 < minDistance) {
            minDistance = l2;
            target = obj;
        }
    }
    return target;
}

float GameSimulation::getLocalSpeedFactorAt(jt::Vector2f const& pos, jt::Vector2f const& dir)
{
    auto const slope = m_world->getSlopeAt(pos, dir);
    if (slope > 0.0f) {
        return 0.997143f - 0.178571f * slope;
    } else if (slope < 0.0f) {
        return 1.0f / (0.997143f + 0.178571f * slope);
    }
    return 1.0f;
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
