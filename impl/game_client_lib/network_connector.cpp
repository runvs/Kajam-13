#include "network_connector.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <iostream>
#include <memory>

void NetworkConnector::doCreate() { }
void NetworkConnector::doUpdate(float const /*elapsed*/) { }
void NetworkConnector::doDraw() const
{

    ImGui::Begin("NetworkConnector");

    ImGui::InputInt("Server Port", &m_serverPort, 1, 1);
    ImGui::InputInt("Client Port", &m_clientPort, 1, 1);
    ImGui::InputText("IP", &m_ip);
    if (ImGui::Button("connect")) {
        m_connection = std::make_shared<ClientNetworkConnection>(m_ip, m_serverPort, m_clientPort);
        m_connection->establishConnection();
    }
    if (ImGui::Button("ping")) {
        if (!m_connection) {
            std::cerr << "no connection\n";
            ImGui::End();
            return;
        }
        m_connection->sendPing();
    }
    ImGui::End();
}
