#include "game_simulation.hpp"
#include "arrow_info.hpp"
#include "simulation_result_data.hpp"
#include "units/server_unit.hpp"
#include <game_properties.hpp>
#include <json_keys.hpp>
#include <math_helper.hpp>
#include <object_properties.hpp>
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
    // TODO check validity of placement
    if (!checkIfUnitIsUnique(props)) {
        return;
    }
    m_unitInformationForRoundStart.push_back(props);
}

float arrowParaboloa(float x, float maxHeight) { return -maxHeight * 4 * (x - x * x); }

void GameSimulation::performSimulation(SimulationResultMessageSender& sender)
{
    auto const timePerUpdate = 0.005f;
    SimulationResultDataForAllFrames allFrames;
    for (auto i = 0u; i != GP::NumberOfStepsPerRound(); ++i) {
        SimulationResultDataForOneFrame currentFrame;

        for (auto& arrow : m_arrows) {
            arrow.age += timePerUpdate;
            float timePercent = arrow.age / arrow.totalTime;
            if (timePercent >= 1) {
                timePercent = 1;
            }
            auto const dif = arrow.endPos - arrow.startPos;
            arrow.currentPos = arrow.startPos + dif * timePercent
                + jt::Vector2f { 0.0f, arrowParaboloa(timePercent, arrow.maxHeight) };

            // check for collision arrow - targets
            for (auto& target : m_simulationObjects) {
                if (!target->isAlive()) {
                    continue;
                }
                if (target->getPlayerID() == arrow.targetPlayerId) {
                    auto const difTargetArrow = target->getPosition() - arrow.currentPos;
                    auto const dist = jt::MathHelper::length(difTargetArrow);
                    if (dist <= 16) {
                        target->takeDamage(arrow.damage);
                        arrow.age = 999999;
                        break;
                    }
                }
            }
            // TODO make arrow follow a parabola
        }

        m_arrows.erase(std::remove_if(m_arrows.begin(), m_arrows.end(),
                           [](auto& arrow) { return arrow.age > arrow.totalTime; }),
            m_arrows.end());

        currentFrame.m_arrows = m_arrows;

        for (auto& obj : m_simulationObjects) {
            obj->update(timePerUpdate, *this);

            auto data = obj->saveState();
            data.ints[jk::simulationTick] = i;

            currentFrame.m_units.push_back(data);
        }

        allFrames.allFrames.push_back(currentFrame);
    }

    m_logger.info("simulation finished");
    sender.sendSimulationResults(allFrames);
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

jt::Vector2f GameSimulation::getTerrainMappedFieldPosition(jt::Vector2f const& pos)
{
    return m_world->getMappedFieldPosition(pos);
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

void GameSimulation::spawnArrow(ArrowInfo const& arrowInfo) { m_arrows.push_back(arrowInfo); }
