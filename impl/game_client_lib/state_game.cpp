﻿#include "state_game.hpp"
#include <game_properties.hpp>
#include <box2dwrapper/box2d_world_impl.hpp>
#include <color/color.hpp>
#include <game_interface.hpp>
#include <screeneffects/vignette.hpp>
#include <shape.hpp>
#include <state_menu.hpp>
#include "server_connection.h"

void StateGame::onCreate() {
    m_world = std::make_shared<jt::Box2DWorldImpl>(jt::Vector2f{0.0f, 0.0f});

    float const w = static_cast<float>(GP::GetWindowSize().x);
    float const h = static_cast<float>(GP::GetWindowSize().y);

    using jt::Shape;

    m_background = std::make_shared<Shape>();
    m_background->makeRect({w, h}, textureManager());
    m_background->setColor(GP::PaletteBackground());
    m_background->setIgnoreCamMovement(true);
    m_background->update(0.0f);

    createPlayer();

    m_vignette = std::make_shared<jt::Vignette>(GP::GetScreenSize());
    add(m_vignette);

    // StateGame will call drawObjects itself.
    setAutoDraw(false);

    m_serverConnection = std::make_shared<ServerConnection>();
    m_serverConnection->setConnection(m_connection);
    add(m_serverConnection);
}

void StateGame::onEnter() {}

void StateGame::createPlayer() {
}

void StateGame::onUpdate(float const elapsed) {
    if (m_running) {
        m_world->step(elapsed, GP::PhysicVelocityIterations(), GP::PhysicPositionIterations());
        // update game logic here

        if (getGame()->input().keyboard()->pressed(jt::KeyCode::LShift)
            && getGame()->input().keyboard()->pressed(jt::KeyCode::Escape)) {
            endGame();
        }
    }

    m_background->update(elapsed);
    m_vignette->update(elapsed);
}

void StateGame::onDraw() const {
    m_background->draw(renderTarget());
    drawObjects();
    m_vignette->draw();
}

void StateGame::endGame() {
    if (m_hasEnded) {
        // trigger this function only once
        return;
    }
    m_hasEnded = true;
    m_running = false;

    getGame()->stateManager().switchToStoredState("menu");
}

std::string StateGame::getName() const { return "State Game"; }

void StateGame::setConnection(std::shared_ptr<ClientNetworkConnection> connection) {
    m_connection = connection;
}
