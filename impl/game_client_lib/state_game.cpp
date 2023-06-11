#include "state_game.hpp"
#include "color/color.hpp"
#include "graphics/drawable_interface.hpp"
#include "simulation_result_data.hpp"
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
#include <state_menu.hpp>
#include <unit_info_collection.hpp>
#include <unit_placement/placement_manager.hpp>
#include <imgui.h>
#include <memory>
#include <stdexcept>

void StateGame::onCreate()
{
    getGame()->gfx().createZLayer(GP::ZLayerUI());
    m_world = std::make_shared<Terrain>();
    m_world_renderer = std::make_shared<TerrainRenderer>(*m_world);
    add(m_world_renderer);

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

    m_arrowShape = jt::dh::createShapeCircle(2, jt::colors::White, textureManager());
    m_arrowShape->setOffset(jt::OffsetMode::CENTER);
}

void StateGame::onEnter() { }

void StateGame::createPlayer() { }

void StateGame::onUpdate(float const elapsed)
{
    if (m_running) {
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
            m_world_renderer->setDrawGrid(true);
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

    m_vignette->update(elapsed);
}

void StateGame::playbackSimulation(float /*elapsed*/)
{
    if (m_simulationResultsForAllFrames.allFrames.size() != 0) {
        if (m_tickId < GP::NumberOfStepsPerRound() - 1) {
            m_tickId++;

        } else {
            transitionPlaybackToPlaceUnits();
        }
        auto const& propertiesForAllUnitsForThisTick
            = m_simulationResultsForAllFrames.allFrames.at(m_tickId);
        placeUnitsForOneTick(propertiesForAllUnitsForThisTick);
    }
}
void StateGame::placeUnitsForOneTick(
    SimulationResultDataForOneFrame const& propertiesForAllUnitsForThisTick)
{
    if (!propertiesForAllUnitsForThisTick.m_playerHP.empty()) {
        getGame()->logger().info("new player hp received");
        m_playerHP = propertiesForAllUnitsForThisTick.m_playerHP;
        if (m_playerHP.at(m_serverConnection->getPlayerId()) <= 0) {
            m_internalState = InternalState::EndLose;
        }
        // TODO win condition
    }
    for (auto const& propsForOneUnit : propertiesForAllUnitsForThisTick.m_units) {

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
        m_world, m_serverConnection->getPlayerId(), m_playerIdDispatcher, m_unitInfo);
    m_placementManager->addFunds(200);
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
    m_world_renderer->setDrawGrid(false);
}
void StateGame::transitionWaitForSimulationResultsToPlayback()
{
    m_simulationResultsForAllFrames = m_serverConnection->getRoundData();
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
    m_placementManager->addFunds(150 + 50 * m_round);
}

void StateGame::placeUnits(float /*elapsed*/)
{
    if (m_internalState != InternalState::PlaceUnits) {
        throw std::logic_error { "placeUnits called when not in placeUnits state" };
    }
}

void StateGame::onDraw() const
{
    m_world_renderer->draw();
    drawObjects();

    if (m_internalState == InternalState::Playback) {
        for (auto const& a : m_simulationResultsForAllFrames.allFrames.at(m_tickId).m_arrows) {
            m_arrowShape->setPosition(a.currentPos);
            m_arrowShape->update(0.0f);
            m_arrowShape->draw(renderTarget());
        }
    }

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
    if (m_internalState != InternalState::WaitForAllPlayers) {
        ImGui::Text("HP Player 0: %i", m_playerHP.at(0));
        ImGui::Text("HP Player 1: %i", m_playerHP.at(1));
    }
    ImGui::Separator();
    ImGui::BeginDisabled(m_internalState != InternalState::PlaceUnits);
    if (ImGui::Button("ready")) {
        transitionPlaceUnitsToWaitForSimulationResults();
    }
    ImGui::EndDisabled();

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
        props.strings[jk::unitAnim] = "idle";
        props.floats[jk::hpCurrent] = props.floats[jk::hpMax];
    }
    placeUnitsForOneTick(propertiesForAllUnitsForThisTick);
}
