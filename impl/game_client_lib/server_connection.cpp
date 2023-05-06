
#include "server_connection.hpp"
#include "message.hpp"
#include "network_properties.hpp"
#include <iostream>
#include <stdexcept>

ServerConnection::~ServerConnection()
{
    if (m_connection) {
        m_connection->setHandleIncomingMessageCallback(nullptr);
    }
}

void ServerConnection::setConnection(std::shared_ptr<ClientNetworkConnection> connection)
{
    if (!connection) {
        throw std::invalid_argument { "ServerConnection passed nullptr" };
    }
    m_connection = connection;
    // TODO move to class function
    m_connection->setHandleIncomingMessageCallback(
        [this](auto const& messageContent, auto const& endpoint) {
            handleMessage(messageContent, endpoint);
        });
    m_connection->sendInitialPing();
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

void ServerConnection::handleMessage(
    std::string const& messageContent, asio::ip::udp::endpoint const& endpoint)
{
    // TODO discard messages not coming from server ip/port
    std::cout << "message content:\n" << messageContent << std::endl;
    nlohmann::json j = nlohmann::json::parse(messageContent);
    Message m = j;
    if (m.type == MessageType::PlayerIdResponse) {
        handleMessagePlayerIdResponse(messageContent);
    }
}

void ServerConnection::handleMessagePlayerIdResponse(std::string const& messageContent)
{
    nlohmann::json j = nlohmann::json::parse(messageContent);
    Message const m = j;
    std::cout << "PlayerIdResponse: " << m.playerId << std::endl;
    m_playerId = m.playerId;
}
