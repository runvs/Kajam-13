#include "state_game.hpp"
#include <box2dwrapper/box2d_world_impl.hpp>
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <input/keyboard/keyboard_defines.hpp>
#include <json_keys.hpp>
#include <message.hpp>
#include <object_group.hpp>
#include <player_id_dispatcher.hpp>
#include <screeneffects/vignette.hpp>
#include <server_connection.hpp>
#include <shape.hpp>
#include <state_menu.hpp>
#include <unit_info_collection.hpp>
#include <unit_placement/placement_manager.hpp>
#include <vector.hpp>
#include <imgui.h>
#include <memory>
#include <stdexcept>

void StateGame::onCreate()
{
    m_world = std::make_shared<jt::Box2DWorldImpl>(jt::Vector2f { 0.0f, 0.0f });

    float const w = static_cast<float>(GP::GetWindowSize().x);
    float const h = static_cast<float>(GP::GetWindowSize().y);

    using jt::Shape;

    m_background = std::make_shared<Shape>();
    m_background->makeRect({ w, h }, textureManager());
    m_background->setColor(GP::PaletteBackground());
    m_background->setIgnoreCamMovement(true);
    m_background->update(0.0f);

    createPlayer();

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
}

void StateGame::onEnter() { }

void StateGame::createPlayer() { }

void StateGame::onUpdate(float const elapsed)
{
    if (m_running) {
        m_world->step(elapsed, GP::PhysicVelocityIterations(), GP::PhysicPositionIterations());
        // update game logic here

        if (getGame()->input().keyboard()->pressed(jt::KeyCode::LShift)
            && getGame()->input().keyboard()->pressed(jt::KeyCode::Escape)) {
            endGame();
        }

        if (m_internalState == InternalState::WaitForAllPlayers) {
            if (m_serverConnection->areAllPlayersConnected()) {
                transitionWaitForPlayersToStartPlaceUnits();
            }
        } else if (m_internalState == InternalState::PlaceUnits) {
            placeUnits(elapsed);
            // transition to "WaitForSimulationResults" is done in onDraw for button push;
        } else if (m_internalState == InternalState::WaitForSimulationResults) {
            if (m_serverConnection->isRoundDataReady()) {
                transitionWaitForSimulationResultsToPlayback();
            }
        } else if (m_internalState == InternalState::Playback) {
            playbackSimulation(elapsed);
        }
    }

    m_background->update(elapsed);
    m_vignette->update(elapsed);
}

void StateGame::playbackSimulation(float elapsed)
{
    if (m_properties.size() != 0) {
        if (m_tickId < GP::NumberOfStepsPerRound() - 1) {
            m_tickId++;

        } else {
            transitionPlaybackToPlaceUnits();
        }
        auto const& propertiesForAllUnitsForThisTick = m_properties.at(m_tickId);
        placeUnitsForOneTick(propertiesForAllUnitsForThisTick);
    }
}
void StateGame::placeUnitsForOneTick(
    std::vector<ObjectProperties> const& propertiesForAllUnitsForThisTick)
{
    for (auto const& propsForOneUnit : propertiesForAllUnitsForThisTick) {

        auto const unitID = propsForOneUnit.ints.at(jk::unitID);
        auto const playerID = propsForOneUnit.ints.at(jk::playerID);
        auto const unitType = propsForOneUnit.strings.at(jk::unitType);
        // TODO make this code a bit nicer
        bool unitFound = false;
        for (auto& u : *m_units) {
            auto unit = u.lock();
            if (unit->getPlayerID() != playerID) {
                continue;
            }
            if (unit->getUnitID() != unitID) {
                continue;
            }
            unit->updateState(propsForOneUnit);
            unitFound = true;
            break;
        }
        // Spawn a new  unit
        if (!unitFound) {
            auto unit = std::make_shared<Unit>(m_unitInfo->getInfoForType(unitType));
            m_units->push_back(unit);
            add(unit);
            unit->setIDs(unitID, playerID);
            unit->updateState(propsForOneUnit);
        }
    }
}

void StateGame::transitionWaitForPlayersToStartPlaceUnits()
{
    m_playerIdDispatcher = std::make_shared<PlayerIdDispatcher>(m_serverConnection->getPlayerId());
    m_unitInfo = std::make_shared<UnitInfoCollection>(
        getGame()->logger(), m_serverConnection->getUnitInfo());

    m_placementManager = std::make_shared<PlacementManager>(
        m_serverConnection->getPlayerId(), m_playerIdDispatcher, m_unitInfo);
    add(m_placementManager);
    m_internalState = InternalState::PlaceUnits;
    m_placementManager->setActive(true);
}

void StateGame::transitionPlaceUnitsToWaitForSimulationResults() const
{
    m_clientEndPlacementData.m_properties = m_placementManager->getPlacedUnits();
    m_placementManager->clearPlacedUnits();
    m_serverConnection->readyRound(m_clientEndPlacementData);
    m_internalState = InternalState::WaitForSimulationResults;
    m_placementManager->setActive(false);
}
void StateGame::transitionWaitForSimulationResultsToPlayback()
{
    m_properties = m_serverConnection->getRoundData();
    m_tickId = 0;
    m_internalState = InternalState::Playback;
}

void StateGame::transitionPlaybackToPlaceUnits()
{
    m_tickId = 0;

    resetAllUnits();
    m_round++;
    getGame()->logger().info("finished playing round simulation", { "StateGame" });
    m_internalState = InternalState::PlaceUnits;
    m_placementManager->setActive(true);
}

void StateGame::placeUnits(float elapsed)
{
    if (m_internalState != InternalState::PlaceUnits) {
        throw std::logic_error { "placeUnits called when not in placeUnits state" };
    }
}

void StateGame::onDraw() const
{
    m_background->draw(renderTarget());
    drawObjects();

    m_vignette->draw();

    ImGui::Begin("Network");
    if (m_internalState == InternalState::WaitForAllPlayers) {
        ImGui::Text("Waiting for players to join");
    } else if (m_internalState == InternalState::PlaceUnits) {
        ImGui::Text("Place units");
    } else if (m_internalState == InternalState::WaitForSimulationResults) {
        ImGui::Text("Waiting for other players to end unit placement");
    } else if (m_internalState == InternalState::Playback) {
        ImGui::Text("Watch the battle evolve");
    }
    ImGui::Separator();
    ImGui::Text("round %i", m_round);

    ImGui::Separator();
    if (ImGui::Button("ready")) {
        transitionPlaceUnitsToWaitForSimulationResults();
    }
    ImGui::End();
}

void StateGame::endGame()
{
    if (m_hasEnded) {
        // trigger this function only once
        return;
    }
    m_hasEnded = true;
    m_running = false;

    getGame()->stateManager().switchToStoredState("menu");
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
    auto propertiesForAllUnitsForThisTick = m_properties.at(0);
    for (auto& props : propertiesForAllUnitsForThisTick) {
        props.strings[jk::unitAnim] = "idle";
        // TODO reset HP to hpmax instead of fixed value.
        props.floats[jk::hpCurrent] = 100.0f;
    }
    placeUnitsForOneTick(propertiesForAllUnitsForThisTick);
}
