#include "state_game.hpp"
#include "client_end_placement_data.hpp"
#include "input/keyboard/keyboard_defines.hpp"
#include "json_keys.hpp"
#include "message.hpp"
#include "object_group.hpp"
#include "server_connection.hpp"
#include "vector.hpp"
#include <box2dwrapper/box2d_world_impl.hpp>
#include <color/color.hpp>
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <screeneffects/vignette.hpp>
#include <shape.hpp>
#include <state_menu.hpp>
#include "imgui.h"
#include "zlib.h"
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
                m_internalState = InternalState::PlaceUnits;
            }
        } else if (m_internalState == InternalState::PlaceUnits) {
            placeUnits();
            // transition to "WaitForSimulationResults" is done in onDraw for button push;
        } else if (m_internalState == InternalState::WaitForSimulationResults) {
            if (m_serverConnection->isRoundDataReady()) {
                m_properties = m_serverConnection->getRoundData();
                m_tickId = 0;
                m_internalState = InternalState::Playback;
            }

        } else if (m_internalState == InternalState::Playback) {
            // TODO only do this in "running mode"
            if (m_properties.size() != 0) {
                if (m_tickId < GP::NumberOfStepsPerRound() - 1) {
                    m_tickId++;
                    // TODO end round and reset to initial setup
                }
                auto const& propertiesForAllUnitsForThisTick = m_properties.at(m_tickId);

                for (auto const& propsForOneUnit : propertiesForAllUnitsForThisTick) {
                    int const unitID = propsForOneUnit.ints.at(jk::unitID);
                    for (auto& u : *m_units) {
                        auto unit = u.lock();
                        if (unit->getUnitID() != unitID) {
                            continue;
                        }
                        unit->updateState(propsForOneUnit);
                        break;
                    }
                }
            }
            // TODO transition to next internal state
        }
    }

    m_background->update(elapsed);
    m_vignette->update(elapsed);
}
void StateGame::placeUnits()
{
    if (m_internalState != InternalState::PlaceUnits) {
        throw std::logic_error { "placeUnits called when not in placeUnits state" };
    }

    if (getGame()->input().keyboard()->justPressed(jt::KeyCode::P)) {
        auto unit = std::make_shared<Unit>();
        m_units->push_back(unit);
        add(unit);
        // TODO only allow placement on player's side
        unit->setPosition(getGame()->input().mouse()->getMousePositionWorld());
        unit->setPlayerID(m_serverConnection->getPlayerId());

        // TODO extend by unit type and other required things
        m_clientEndPlacementData.m_properties.push_back(unit->saveState());
    }

    // TODO remove this
    if (getGame()->input().keyboard()->justPressed(jt::KeyCode::O)) {
        auto unit = std::make_shared<Unit>();
        m_units->push_back(unit);
        add(unit);
        unit->setPosition(getGame()->input().mouse()->getMousePositionWorld());
        unit->setPlayerID(m_serverConnection->getPlayerId() + 1);

        // TODO extend by unit type and other required things
        m_clientEndPlacementData.m_properties.push_back(unit->saveState());
    }
}

void StateGame::onDraw() const
{
    m_background->draw(renderTarget());
    drawObjects();
    m_vignette->draw();

    ImGui::Begin("Network");
    ImGui::Text("round %i", m_round);
    ImGui::Separator();
    ImGui::Separator();
    if (ImGui::Button("ready")) {
        m_serverConnection->readyRound(m_clientEndPlacementData);
        m_internalState = InternalState::WaitForSimulationResults;
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
