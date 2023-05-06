
#include "server_connection.hpp"
#include "message.hpp"
#include "network_properties.hpp"
#include <iostream>
#include <stdexcept>

void ServerConnection::setConnection(std::shared_ptr<ClientNetworkConnection> connection)
{
    if (!connection) {
        throw std::invalid_argument { "ServerConnection passed nullptr" };
    }
    m_connection = connection;
    // TODO move to class function
    m_connection->setHandleIncomingMessageCallback(
        [this](auto const& messageContent, auto const& endpoint) {
            std::cout << "message content:\n" << messageContent << std::endl;
            nlohmann::json j = nlohmann::json::parse(messageContent);
            Message m = j;
            if (m.type == MessageType::PlayerIdResponse) {
                std::cout << "PlayerIdResponse: ";
                auto j_data = nlohmann::json::parse(m.data);
                j_data.at("playerId").get_to(m_playerId);
                std::cout << m_playerId << std::endl;
            }
        });
}

std::shared_ptr<ClientNetworkConnection> ServerConnection::getConnection() { return m_connection; }

void ServerConnection::doUpdate(const float elapsed)
{
    if (!m_connection) {
        return;
    }
    m_alivePingTimer -= elapsed;
    if (m_alivePingTimer <= 0) {
        m_alivePingTimer += NetworkProperties::AlivePingTimer();
        m_connection->sendAlivePing(m_playerId);
    }
}
void ServerConnection::readyRound(ClientEndPlacementData const& data)
{
    Message m;
    m.type = MessageType::RoundReady;
    nlohmann::json j = data;
    m.data = j.dump();
    m_connection->sendMessage(m);
}
ServerConnection::~ServerConnection()
{
    if (m_connection) {
        m_connection->setHandleIncomingMessageCallback(nullptr);
    }
}
