#include "game_simulation.hpp"
#include "arrow_info.hpp"
#include "box2dwrapper/box2d_world_impl.hpp"
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
    , m_world { std::make_shared<Terrain>() }
{
}

void GameSimulation::prepareSimulationForNewRound()
{
    m_simulationObjects.clear();
    m_b2World = std::make_shared<jt::Box2DWorldImpl>(jt::Vector2f { 0.0f, 0.0f });

    for (auto const& props : m_unitInformationForRoundStart) {
        auto obj = std::make_shared<ServerUnit>(
            m_logger, m_unitInfos.getInfoForType(props.strings.at(jk::unitType)), m_b2World);
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

        m_b2World->step(timePerUpdate, 10, 10);

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
    if (slope >= 0.0f && slope < 61.0f) {
        // found acceptable curve with following input:
        //  0 -> 1
        // 10 -> 0.95
        // 20 -> 0.85
        // 30 -> 0.7
        // 45 -> 0.5
        // 60 -> 0.2
        return -0.000166516f * slope * slope - 0.00397695f * slope + 0.998285f;
    }
    if (slope > -61.0f && slope < 0.0f) {
        // found acceptable curve with following input:
        //   0 -> 1
        // -10 -> 1.1
        // -20 -> 1.5
        // -30 -> 3
        // -45 -> 5
        // -60 -> 8
        return 0.00190862f * slope * slope - 0.00435245f * slope + 0.912302f;
    }
    return 0.0f;
}

jt::Vector2f GameSimulation::getTerrainMappedFieldPosition(jt::Vector2f const& pos)
{
    return m_world->getMappedFieldPosition(pos);
}

float GameSimulation::getTerrainMappedFieldHeight(jt::Vector2f const& pos)
{
    return m_world->getFieldHeight(pos);
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
