#include "server_connector.hpp"
#include <game_interface.hpp>
#include <message.hpp>
#include <state_game.hpp>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

ServerConnector::ServerConnector(std::shared_ptr<CompressorInterface> compressor)
    : m_compressor { compressor }
{
}

void ServerConnector::doCreate() { }

void ServerConnector::doUpdate(float const /*elapsed*/) { }

void ServerConnector::doDraw() const
{
    ImGui::Begin("ServerConnector");

    ImGui::InputInt("Server Port", &m_serverPort, 1, 1);
    ImGui::InputInt("Client Port", &m_clientPort, 1, 1);
    ImGui::InputText("IP", &m_ip);
    ImGui::Checkbox("Add bot as blue player (left)", &m_addBotAsPlayerZero);
    ImGui::Checkbox("Add bot as red player (right)", &m_addBotAsPlayerOne);
    if (ImGui::Button("connect")) {
        m_connection = std::make_shared<ClientNetworkConnection>(
            m_ip, m_serverPort, m_clientPort, getGame()->logger(), m_compressor);
        m_connection->establishConnection();

        auto const state = std::make_shared<StateGame>();
        state->setConnection(getConnection(), m_addBotAsPlayerZero, m_addBotAsPlayerOne);
        getGame()->stateManager().switchState(state);
    }
    ImGui::End();
}

std::shared_ptr<ClientNetworkConnection> ServerConnector::getConnection() const
{
    return m_connection;
}
