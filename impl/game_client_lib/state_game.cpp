#include "state_game.hpp"
#include <color/color.hpp>
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <graphics/drawable_interface.hpp>
#include <input/keyboard/keyboard_defines.hpp>
#include <internal_state/internal_state_manager.hpp>
#include <message.hpp>
#include <object_group.hpp>
#include <particle_system.hpp>
#include <player_id_dispatcher.hpp>
#include <random/random.hpp>
#include <screeneffects/vignette.hpp>
#include <server_connection.hpp>
#include <shape.hpp>
#include <simulation_result_data.hpp>
#include <state_menu.hpp>
#include <tweens/tween_alpha.hpp>
#include <unit_info_collection.hpp>
#include <unit_placement/placement_manager.hpp>
#include <vector.hpp>
#include <imgui.h>
#include <memory>

void StateGame::onCreate()
{
    getGame()->gfx().createZLayer(GP::ZLayerUI());
    m_world = std::make_shared<Terrain>();
    m_terrainRenderer = std::make_shared<TerrainRenderer>(*m_world);
    add(m_terrainRenderer);

    m_internalStateManager = std::make_shared<InternalStateManager>();

    m_units = std::make_shared<jt::ObjectGroup<Unit>>();
    add(m_units);

    m_vignette = std::make_shared<jt::Vignette>(GP::GetScreenSize());
    add(m_vignette);

    // StateGame will call drawObjects itself.
    setAutoDraw(false);

    if (m_addBotAsPlayerZero) {
        Message m;
        m.type = MessageType::AddBot;
        m_connection->sendMessage(m);
    }
    m_serverConnection = std::make_shared<ServerConnection>(getGame()->logger());
    m_serverConnection->setConnection(m_connection);
    add(m_serverConnection);
    if (m_addBotAsPlayerOne) {
        if (m_addBotAsPlayerZero) {
            getGame()->logger().warning("Create game with two bots", { "StateGame", "Bots" });
        }
        Message m;
        m.type = MessageType::AddBot;
        m_connection->sendMessage(m);
    }

    m_arrowShape = jt::dh::createShapeCircle(1, jt::colors::White, textureManager());
    m_arrowShape->setOffset(jt::OffsetMode::CENTER);

    m_clouds = std::make_shared<jt::Clouds>(jt::Vector2f { 4.0f, 2.0f });
    add(m_clouds);

    m_birds = std::make_shared<jt::ObjectGroup<Bird>>();

    for (auto i = 0; i != 10; ++i) {
        auto b = std::make_shared<Bird>();
        m_birds->push_back(b);
        add(b);
        b->setPosition(jt::Random::getRandomPointIn(GP::GetScreenSize()));
    }

    m_explosionParticles = jt::ParticleSystem<jt::Shape, 50>::createPS(
        [this]() {
            std::shared_ptr<jt::Shape> shape
                = jt::dh::createShapeCircle(4, jt::colors::White, textureManager());
            shape->setPosition(jt::Vector2f { -5000.0f, -5000.0f });
            return shape;
        },
        [this](auto shape, auto const& pos) {
            shape->setPosition(pos);
            auto const tw = jt::TweenAlpha::create(shape, 0.5f, 255, 0);
            add(tw);
        });
    add(m_explosionParticles);
}

void StateGame::onEnter() { }

void StateGame::onUpdate(float const elapsed)
{
    if (m_running) {
        m_explosionParticles->update(elapsed);
        // update game logic here

        if (getGame()->input().keyboard()->pressed(jt::KeyCode::LShift)
            && getGame()->input().keyboard()->pressed(jt::KeyCode::Escape)) {
            endGame();
        }

        m_internalStateManager->getActiveState()->update(*this, elapsed);
    }
}

void StateGame::playbackSimulation(float /*elapsed*/)
{
    if (!m_simulationResultsForAllFrames.allFrames.empty()) {
        if (m_tickId < m_simulationResultsForAllFrames.allFrames.size() - 1) {
            m_tickId++;
        } else {
            getStateManager()->switchToState(InternalState::PlaceUnits, *this);
        }
        auto const& propertiesForAllUnitsForThisFrame
            = m_simulationResultsForAllFrames.allFrames.at(m_tickId);
        playbackOneFrame(propertiesForAllUnitsForThisFrame);
    }
}

void StateGame::playbackOneFrame(SimulationResultDataForOneFrame const& currentFrame)
{
    if (!currentFrame.m_playerHP.empty()) {
        getGame()->logger().info("new player hp received");
        m_playerHP = currentFrame.m_playerHP;
        auto const thisPlayerId = m_serverConnection->getPlayerId();
        auto const otherPlayerId = ((thisPlayerId == 0) ? 1 : 0);
        if (m_playerHP.at(thisPlayerId) <= 0) {
            getStateManager()->switchToState(InternalState::EndLose, *this);
        } else if (m_playerHP.at(otherPlayerId) <= 0) {
            getStateManager()->switchToState(InternalState::EndWin, *this);
        }
    }

    for (auto const& propsForOneUnit : currentFrame.m_units) {
        std::shared_ptr<Unit> foundUnit = findOrCreateUnit(
            propsForOneUnit.playerID, propsForOneUnit.unitID, propsForOneUnit.unitType);
        foundUnit->updateState(propsForOneUnit);
    }

    for (auto const& expl : currentFrame.m_explosions) {
        getGame()->gfx().camera().shake(0.5f, 5);
        for (auto i = 0; i != 10; ++i) {
            auto const pos = jt::Random::getRandomPointInCircle(expl.radius) + expl.position;
            m_explosionParticles->fire(1, pos);
        }
    }
}

std::shared_ptr<Unit> StateGame::findOrCreateUnit(int pid, int uid, const std::string& type)
{
    for (auto const& u : *m_units) {
        auto currentUnit = u.lock();
        if (currentUnit->getPlayerID() != pid) {
            continue;
        }
        if (currentUnit->getUnitID() != uid) {
            continue;
        }
        return currentUnit;
    }

    // Spawn a new  unit
    std::shared_ptr<Unit> newUnit = std::make_shared<Unit>(m_unitInfo->getInfoForType(type));
    m_units->push_back(newUnit);
    add(newUnit);
    newUnit->setIDs(uid, pid);
    return newUnit;
}

void StateGame::transitionWaitForPlayersToSelectStartingUnits()
{
    m_playerIdDispatcher = std::make_shared<PlayerIdDispatcher>(m_serverConnection->getPlayerId());
    m_unitInfo = std::make_shared<UnitInfoCollection>(
        getGame()->logger(), m_serverConnection->getUnitInfo());

    m_placementManager = std::make_shared<PlacementManager>(
        m_world, m_serverConnection->getPlayerId(), m_playerIdDispatcher, m_unitInfo);
    add(m_placementManager);
}

void StateGame::transitionWaitForSimulationResultsToPlayback()
{
    m_simulationResultsForAllFrames = m_serverConnection->getRoundData();
    m_tickId = 0;
}

void StateGame::transitionPlaybackToPlaceUnits()
{
    m_tickId = 0;
    resetAllUnits();
    m_round++;
    getGame()->logger().info("finished playing round simulation", { "StateGame" });
}

void StateGame::onDraw() const
{
    m_terrainRenderer->draw();

    // first draw all dead units
    for (auto const& u : *m_units) {
        auto const lockedUnit = u.lock();
        if (!lockedUnit->isUnitAlive()) {
            lockedUnit->draw();
        }
    }

    for (auto const& b : *m_birds) {
        b.lock()->draw();
    }

    // then draw all alive units
    for (auto const& u : *m_units) {
        auto const lockedUnit = u.lock();
        if (lockedUnit->isUnitAlive()) {
            lockedUnit->update(0.0f);
            lockedUnit->draw();
        }
    }

    // ouch
    m_internalStateManager->getActiveState()->draw(const_cast<StateGame&>(*this));

    m_explosionParticles->draw();

    m_clouds->draw();
    m_vignette->draw();

    ImGui::Begin("State");
    if (m_internalStateManager->getActiveStateE() == InternalState::WaitForAllPlayers) {
        ImGui::Text("Waiting for players to join");
    } else if (m_internalStateManager->getActiveStateE() == InternalState::PlaceUnits) {
        ImGui::Text("Place units");
    } else if (m_internalStateManager->getActiveStateE()
        == InternalState::WaitForSimulationResults) {
        ImGui::Text("Waiting for other players to end unit placement");
    } else if (m_internalStateManager->getActiveStateE() == InternalState::Playback) {
        ImGui::Text("Watch the battle evolve");
    }
    ImGui::Separator();
    ImGui::Text("round %i", m_round);
    if (m_internalStateManager->getActiveStateE() != InternalState::WaitForAllPlayers) {
        ImGui::Text("HP Player 0: %i", m_playerHP.at(0));
        ImGui::Text("HP Player 1: %i", m_playerHP.at(1));
    }

    ImGui::End();
}
void StateGame::drawArrows() const
{
    for (auto const& a : m_simulationResultsForAllFrames.allFrames.at(m_tickId).m_arrows) {
        m_arrowShape->setPosition(a.currentPos);
        m_arrowShape->update(0.0f);
        m_arrowShape->setScale(jt::Vector2f { a.arrowScale, a.arrowScale });
        m_arrowShape->draw(renderTarget());
    }
}

void StateGame::endGame()
{
    if (m_hasEnded) {
        // trigger this function only once
        return;
    }
    m_hasEnded = true;
    m_running = false;

    getGame()->stateManager().switchState(std::make_shared<StateMenu>());
}

std::string StateGame::getName() const { return "State Game"; }

void StateGame::setConnection(
    std::shared_ptr<ClientNetworkConnection> connection, bool botAsPlayerZero, bool botAsPlayerOne)
{
    m_connection = connection;
    m_addBotAsPlayerZero = botAsPlayerZero;
    m_addBotAsPlayerOne = botAsPlayerOne;
}

void StateGame::resetAllUnits()
{
    auto propertiesForAllUnitsForThisTick = m_simulationResultsForAllFrames.allFrames.at(0);
    for (auto& props : propertiesForAllUnitsForThisTick.m_units) {
        props.unitAnim = "idle";
        props.hpCurrent = props.hpMax;
    }
    playbackOneFrame(propertiesForAllUnitsForThisTick);
}

std::shared_ptr<InternalStateManager> StateGame::getStateManager()
{
    return m_internalStateManager;
}
std::shared_ptr<ServerConnection> StateGame::getServerConnection() { return m_serverConnection; }
std::shared_ptr<TerrainRenderer> StateGame::getTerrainRenderer() { return m_terrainRenderer; }
std::shared_ptr<PlacementManager> StateGame::getPlacementManager() { return m_placementManager; }
std::shared_ptr<UnitInfoCollection> StateGame::getUnitInfo() { return m_unitInfo; }
int StateGame::getRound() { return m_round; }
std::shared_ptr<jt::ObjectGroup<Unit>> StateGame::getUnits() { return m_units; }
std::shared_ptr<jt::ObjectGroup<Bird>> StateGame::getBirds() { return m_birds; }
void StateGame::flashUnitsForUpgrade(const std::string& unitType)
{
    for (auto& u : *m_units) {
        auto unit = u.lock();
        if (unit->getPlayerID() != m_serverConnection->getPlayerId()) {
            continue;
        }
        if (unit->getInfo().type == unitType) {
            unit->flash();
        }
    }
    m_placementManager->flashForUpgrade(unitType);
}
