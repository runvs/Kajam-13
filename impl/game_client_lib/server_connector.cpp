#include "server_connector.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <iostream>
#include <memory>

void ServerConnector::doCreate() { }

void ServerConnector::doUpdate(float const /*elapsed*/) { }

void ServerConnector::doDraw() const
{
    ImGui::Begin("ServerConnector");

    ImGui::InputInt("Server Port", &m_serverPort, 1, 1);
    ImGui::InputInt("Client Port", &m_clientPort, 1, 1);
    ImGui::InputText("IP", &m_ip);
    if (ImGui::Button("connect")) {
        m_connection = std::make_shared<ClientNetworkConnection>(m_ip, m_serverPort, m_clientPort);
        m_connection->establishConnection();
    }
    //    if (ImGui::Button("ping")) {
    //        if (!m_connection) {
    //            std::cerr << "no connection\n";
    //            ImGui::End();
    //            return;
    //        }
    //        m_connection->sendInitialPing();
    //    }
    ImGui::End();
}

std::shared_ptr<ClientNetworkConnection> ServerConnector::getConnection() const
{
    return m_connection;
}
