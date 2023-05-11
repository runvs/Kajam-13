#include "state_game.hpp"
#include "client_end_placement_data.hpp"
#include "input/keyboard/keyboard_defines.hpp"
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

    m_unit = std::make_shared<Unit>();
    add(m_unit);

    m_vignette = std::make_shared<jt::Vignette>(GP::GetScreenSize());
    add(m_vignette);

    // StateGame will call drawObjects itself.
    setAutoDraw(false);

    m_serverConnection = std::make_shared<ServerConnection>(getGame()->logger());
    m_serverConnection->setConnection(m_connection);
    add(m_serverConnection);
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

        // TODO only do this in "placement mode"
        if (getGame()->input().keyboard()->justPressed(jt::KeyCode::P)) {

            m_unit->setPosition(getGame()->input().mouse()->getMousePositionWorld());
            // TODO extend by unit type and other required things
            m_clientEndPlacementData.m_position
                = getGame()->input().mouse()->getMousePositionWorld();
        }

        // Waiting state for data
        if (m_serverConnection->isRoundDataReady()) {
            m_properties = m_serverConnection->getRoundData();
            m_tickId = 0;
        }

        // TODO only do this in "running mode"
        if (m_properties.size() != 0) {

            if (m_tickId < GP::NumberOfStepsPerRound() - 1) {
                m_tickId++;
            }
            jt::Vector2f const pos = jt::Vector2f { m_properties.at(m_tickId).floats.at("posX"),
                m_properties.at(m_tickId).floats.at("posY") };
            m_unit->setPosition(pos);
        }
    }

    m_background->update(elapsed);
    m_vignette->update(elapsed);
}

void StateGame::onDraw() const
{
    m_background->draw(renderTarget());
    drawObjects();
    m_vignette->draw();

    ImGui::Begin("Network");
    ImGui::Text("round %i", m_round);
    ImGui::Separator();
    ImGui::Text("pos: %.1f, %.1f", m_clientEndPlacementData.m_position.x,
        m_clientEndPlacementData.m_position.y);
    ImGui::Separator();
    if (ImGui::Button("ready")) {
        m_serverConnection->readyRound(m_clientEndPlacementData);
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

void StateGame::setConnection(std::shared_ptr<ClientNetworkConnection> connection)
{
    m_connection = connection;
}
