#include "game_simulation.hpp"
#include "system_helper.hpp"
#include "validity_checker.hpp"
#include <box2dwrapper/box2d_world_impl.hpp>
#include <game_properties.hpp>
#include <math_helper.hpp>
#include <network_data/arrow_info.hpp>
#include <network_data/explosion_info.hpp>
#include <simulation_object_interface.hpp>
#include <simulation_result_data.hpp>
#include <units/server_unit.hpp>
#include <memory>
#include <string>

namespace {

std::string getRandomMapName()
{
    // clang-format off
    static std::vector<std::string> mapList {
        "assets/maps/map_de_dust_2.json",
        "assets/maps/map_over_the_hills.json",
        "assets/maps/map_plains.json",
        "assets/maps/map_valley_of_death.json"
    };
    // clang-format on
    return *jt::SystemHelper::select_randomly(mapList.cbegin(), mapList.cend());
}

} // namespace

GameSimulation::GameSimulation(jt::LoggerInterface& logger, UnitInfoCollection& unitInfos)
    : m_logger { logger }
    , m_unitInfos { unitInfos }
    , m_world { std::make_shared<Terrain>() }
{
}

void GameSimulation::rollNewMap()
{
    const auto name = getRandomMapName();
    m_logger.info("new map: " + name);
    m_world = std::make_shared<Terrain>(name);
}

void GameSimulation::prepareSimulationForNewRound()
{
    m_arrows.clear();
    m_arrowsToBeSpawned.clear();
    m_scheduledCloseCombatAttacks.clear();
    m_simulationObjects.clear();
    m_b2World = std::make_shared<jt::Box2DWorldImpl>(jt::Vector2f { 0.0f, 0.0f });

    for (auto& props : m_unitInformationForRoundStart) {
        auto obj = std::make_shared<ServerUnit>(
            m_logger, m_unitInfos.getInfoForType(props.unitClientToServerData.unitType), m_b2World);
        obj->setRoundStartState(&props);
        obj->applyUpgrades(m_unitUpgrades);
        m_simulationObjects.emplace_back(std::move(obj));
    }
}

void GameSimulation::addUnit(UnitClientToServerData const& unitData)
{
    if (!ValidityChecker::checkIfUnitIDsAreUnique(m_unitInformationForRoundStart, unitData)) {
        m_logger.warning("Adding a unit that is already present in the game simulation");
        return;
    }

    if (!ValidityChecker::checkIfUnitPlacementIsValid(unitData)) {
        m_logger.warning("Adding a unit in an invalid place");
        return;
    }

    UnitServerRoundStartData roundStartData;
    roundStartData.unitClientToServerData = unitData;

    m_unitInformationForRoundStart.push_back(roundStartData);
}

float arrowParabola(float x, float maxHeight) { return -maxHeight * 4 * (x - x * x); }

void GameSimulation::performSimulation(SimulationResultSenderInterface& sender)
{
    auto const timePerUpdate = GP::TimePerSimulationUpdate();
    SimulationResultDataForAllFrames allFrames;
    for (auto i = 0u; i != GP::MaxNumberOfStepsPerRound(); ++i) {
        SimulationResultDataForOneFrame currentFrame;

        m_b2World->step(timePerUpdate, 10, 10);

        handleScheduledAttacks(timePerUpdate);
        handleArrowsToBeSpawned(timePerUpdate);

        handleArrows(timePerUpdate, currentFrame);

        currentFrame.m_arrows = m_arrows;

        for (auto& obj : m_simulationObjects) {
            obj->update(timePerUpdate, *this);

            auto data = obj->saveState();

            currentFrame.m_units.push_back(data);
        }

        currentFrame.m_frameId = i;
        bool isLastFrame = checkIsLastFrame(i);

        if (isLastFrame) {
            for (auto& obj : m_simulationObjects) {
                if (!obj->isAlive()) {
                    continue;
                }
                auto const objPid = obj->getPlayerID();
                auto const playerPid = ((objPid == 0) ? 1 : 0);
                m_playerHp[playerPid] -= obj->getCost();
            }
            currentFrame.m_playerHP = m_playerHp;
        }
        allFrames.allFrames.push_back(currentFrame);
        if (isLastFrame) {
            break;
        }
    }

    m_logger.info("simulation finished");
    sender.sendSimulationResults(allFrames);
    m_logger.info("sending of simulation results done");
}

bool GameSimulation::checkIsLastFrame(unsigned int i)
{
    bool isLastFrame = (i == GP::MaxNumberOfStepsPerRound() - 1);
    bool playerZeroAlive { false };
    bool playerOneAlive { false };

    for (auto& obj : m_simulationObjects) {
        if (!obj->isAlive()) {
            continue;
        }
        if (obj->getPlayerID() == 0) {
            playerZeroAlive = true;
        } else {
            playerOneAlive = true;
        }
        if (playerZeroAlive && playerOneAlive) {
            break;
        }
    }
    if (!playerZeroAlive || !playerOneAlive) {
        isLastFrame = true;
    }
    return isLastFrame;
}

void GameSimulation::handleArrows(
    float timePerUpdate, SimulationResultDataForOneFrame& currentFrame)
{
    for (auto& arrow : m_arrows) {
        arrow.age += timePerUpdate;
        float timePercent = arrow.age / arrow.totalTime;
        if (timePercent >= 1) {
            timePercent = 1;
        }
        auto const dif = arrow.endPos - arrow.startPos;
        arrow.currentPos = arrow.startPos + dif * timePercent
            + jt::Vector2f { 0.0f, arrowParabola(timePercent, arrow.maxHeight) };

        if (arrow.splashRadius <= 0) {
            // check for single collision arrow - targets
            for (auto& target : m_simulationObjects) {
                if (!target->isAlive()) {
                    continue;
                }

                if (target->getPlayerID() == arrow.targetPlayerId) {
                    auto const difTargetArrow
                        = target->getPosition() - arrow.currentPos + jt::Vector2f { 4.0f, 4.0f };
                    auto const dist = jt::MathHelper::length(difTargetArrow);

                    if (dist <= 16) {

                        target->takeDamage(arrow.damage);
                        if (!target->isAlive()) {
                            // kill
                            m_logger.verbose("arrow kill", { "GameSimulation" });
                            auto const exp = target->getUnitInfoFull().experienceGainWhenKilled;
                            for (auto& u : m_simulationObjects) {
                                bool const correctUId = u->getUnitID() == arrow.shooterUnitId;
                                bool const correctPId = u->getPlayerID() == arrow.shooterPlayerId;
                                if (correctPId && correctUId) {
                                    u->gainExperience(exp);
                                    m_logger.verbose(
                                        "gain exp from arrow kill: " + std::to_string(exp),
                                        { "GameSimulation" });
                                    break;
                                }
                            }
                        }
                        arrow.age = 999999;
                        break;
                    }
                }
            }
        } else {
            if (arrow.age >= arrow.totalTime) {

                ExplosionInfo explosion;
                explosion.position = arrow.currentPos;
                explosion.radius = arrow.splashRadius;
                currentFrame.m_explosions.push_back(explosion);
                // damage all enemies in splashrange
                for (auto& target : m_simulationObjects) {
                    if (!target->isAlive()) {
                        continue;
                    }

                    if (target->getPlayerID() == arrow.targetPlayerId) {
                        auto const difTargetArrow = target->getPosition() - arrow.currentPos
                            + jt::Vector2f { 4.0f, 4.0f };
                        auto const dist = jt::MathHelper::length(difTargetArrow);

                        if (dist <= arrow.splashRadius) {
                            target->takeDamage(arrow.damage);
                            if (!target->isAlive()) {
                                // kill
                                m_logger.verbose("cannonball kill", { "GameSimulation" });
                                auto const exp = target->getUnitInfoFull().experienceGainWhenKilled;
                                for (auto& u : m_simulationObjects) {
                                    bool const correctUId = u->getUnitID() == arrow.shooterUnitId;
                                    bool const correctPId
                                        = u->getPlayerID() == arrow.shooterPlayerId;
                                    if (correctPId && correctUId) {
                                        u->gainExperience(exp);
                                        m_logger.verbose(
                                            "gain exp from cannonball kill: " + std::to_string(exp),
                                            { "GameSimulation" });
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    std::erase_if(m_arrows, [](auto const& arrow) { return arrow.age > arrow.totalTime; });
}

void GameSimulation::handleArrowsToBeSpawned(float timePerUpdate)
{
    for (auto& kvp : m_arrowsToBeSpawned) {
        kvp.first -= timePerUpdate;
        if (kvp.first <= 0) {
            auto const unit = getUnit(kvp.second.shooterPlayerId, kvp.second.shooterUnitId);
            if (!unit) {
                continue;
            }
            if (!unit->isAlive()) {
                continue;
            }
            m_arrows.push_back(kvp.second);
        }
    }

    std::erase_if(m_arrowsToBeSpawned, [](auto const& kvp) { return kvp.first <= 0; });
}

void GameSimulation::handleScheduledAttacks(float timePerUpdate)
{
    for (auto& kvp : m_scheduledCloseCombatAttacks) {
        kvp.first -= timePerUpdate;
        if (!kvp.second.attacker->isAlive()) {
            continue;
        }
        if (!kvp.second.attacked->isAlive()) {
            continue;
        }

        if (kvp.first <= 0) {
            kvp.second.attacked->takeDamage(kvp.second.damage);
            if (!kvp.second.attacked->isAlive()) {
                auto const exp = kvp.second.attacked->getUnitInfoFull().experienceGainWhenKilled;
                kvp.second.attacker->gainExperience(exp);
            }
        }
    }

    std::erase_if(m_scheduledCloseCombatAttacks, [](auto const& kvp) { return kvp.first <= 0; });
}

std::weak_ptr<SimulationObjectInterface> GameSimulation::getClosestTargetTo(
    const jt::Vector2f& pos, int playerId)
{
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

float GameSimulation::getLocalSlope(jt::Vector2f const& pos, jt::Vector2f const& dir)
{
    return m_world->getSlopeAt(pos, dir);
}

jt::Vector2f GameSimulation::getTerrainMappedFieldPosition(jt::Vector2f const& pos, int& x, int& y)
{
    return m_world->getMappedFieldPosition(pos, x, y);
}

float GameSimulation::getTerrainMappedFieldHeight(jt::Vector2f const& pos)
{
    return m_world->getFieldHeight(pos);
}

void GameSimulation::spawnArrow(ArrowInfo const& arrowInfo, float delay)
{
    m_arrowsToBeSpawned.push_back(std::make_pair(delay, arrowInfo));
}

void GameSimulation::clear()
{
    m_arrows.clear();
    m_arrowsToBeSpawned.clear();
    m_scheduledCloseCombatAttacks.clear();
    m_unitInformationForRoundStart.clear();
    m_simulationObjects.clear();
    m_unitUpgrades.clear();
    m_playerHp = { { 0, GP::InitialPlayerHP() }, { 1, GP::InitialPlayerHP() } };
    m_world = std::make_shared<Terrain>();
}

void GameSimulation::addUnitUpgrade(const UpgradeUnitData& upg)
{
    m_unitUpgrades.emplace_back(upg);
}

void GameSimulation::scheduleAttack(CloseCombatInfo const& info, float delay)
{
    m_scheduledCloseCombatAttacks.push_back(std::make_pair(delay, info));
}

std::shared_ptr<SimulationObjectInterface> GameSimulation::getUnit(int pid, int uid)
{
    for (auto const& obj : m_simulationObjects) {
        if (obj->getPlayerID() == pid && obj->getUnitID() == uid) {
            return obj;
        }
    }
    return nullptr;
}
