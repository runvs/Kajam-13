#include "server_connector.hpp"
#include <game_interface.hpp>
#include <message.hpp>
#include <state_game.hpp>
#include <fstream>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace {

void saveIp(std::string const& ip)
{
    std::ofstream file { "ip.txt", std::ios_base::trunc };
    if (file.is_open()) {
        file << ip;
    }
}

std::string loadIp()
{
    std::ifstream file { "ip.txt" };
    std::string ip;
    if (file.is_open()) {
        file >> ip;
    }
    if (ip.empty()) {
        return NetworkProperties::DefaultIPAddress();
    }
    return ip;
}

} // namespace

ServerConnector::ServerConnector(std::shared_ptr<CompressorInterface> compressor)
    : m_compressor { compressor }
{
}

void ServerConnector::doCreate() { m_ip = loadIp(); }

void ServerConnector::doUpdate(float const /*elapsed*/) { }

void ServerConnector::doDraw() const
{
    ImGui::Begin("ServerConnector");

    ImGui::InputInt("Server Port", &m_serverPort, 1, 1);
    ImGui::InputInt("Client Port", &m_clientPort, 1, 1);
    if (ImGui::InputText(
            "IP", &m_ip, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank)) {
        connect();
    }
    ImGui::Checkbox("Add bot as blue player (left)", &m_addBotAsPlayerZero);
    ImGui::Checkbox("Add bot as red player (right)", &m_addBotAsPlayerOne);
    if (ImGui::Button("connect")) {
        connect();
    }
    ImGui::End();
}

void ServerConnector::connect() const
{
    saveIp(m_ip);
    m_connection = std::make_shared<ClientNetworkConnection>(
        m_ip, m_serverPort, m_clientPort, getGame()->logger(), m_compressor);
    m_connection->establishConnection();

    auto const state = std::make_shared<StateGame>();
    state->setConnection(getConnection(), m_addBotAsPlayerZero, m_addBotAsPlayerOne);
    getGame()->stateManager().switchState(state);
}

std::shared_ptr<ClientNetworkConnection> ServerConnector::getConnection() const
{
    return m_connection;
}
