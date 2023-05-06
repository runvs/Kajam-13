#include "game_server.hpp"
#include "client_end_placement_data.hpp"
#include "message.hpp"
#include "network_properties.hpp"
#include "player_info.hpp"
#include <nlohmann.hpp>
#include <iostream>

GameServer::GameServer()
{
    m_connection.setHandleIncomingMessageCallback(
        // TODO move to class function
        [this](auto const& messageContent, auto endpoint) {
            handleMessage(messageContent, endpoint);
        });
}

void GameServer::update(float elapsed)
{
    m_connection.update();
    // Note: remove_if does not work for map
    for (auto it = m_playerData.begin(); it != m_playerData.end();) {
        it->second.timeSinceLastPing += elapsed;
        //        std::cout << "age: " << it->second << std::endl;

        if (it->second.timeSinceLastPing >= 5.0f * NetworkProperties::AlivePingTimer()) {
            std::cout << "erase endpoint: " << it->second.endpoint.address() << "\n";
            it = m_playerData.erase(it);

        } else {
            ++it;
        }
    }
}
void GameServer::handleMessage(
    const std::string& messageContent, const asio::ip::udp::endpoint& endpoint)
{
    std::cout << "message content:\n" << messageContent << std::endl;
    nlohmann::json j = nlohmann::json::parse(messageContent);
    Message m = j;
    if (m.type == MessageType::InitialPing) {
        handleMessageInitialPing(messageContent, endpoint);
    } else if (m.type == MessageType::StayAlivePing) {
        handleMessageStayAlivePing(messageContent, endpoint);
    } else if (m.type == MessageType::RoundReady) {
        handleMessageRoundReady(messageContent, endpoint);
    } else {
        discard(messageContent, endpoint);
    }
}
void GameServer::handleMessageInitialPing(
    std::string const& /*messageContent*/, asio::ip::udp::endpoint const& endpoint)
{
    std::cout << "initial ping received\n";
    // TODO check if player is already registered
    // TODO check if already full
    m_playerData[m_connectedPlayers] = PlayerInfo {};
    m_playerData[m_connectedPlayers].timeSinceLastPing = 0.0f;
    m_playerData[m_connectedPlayers].endpoint = endpoint;

    Message ret;
    ret.type = MessageType::PlayerIdResponse;
    ret.data = nlohmann::json { { "playerId", m_connectedPlayers } }.dump();
    m_connection.sendMessage(ret, endpoint);

    std::cout << "assigned new player ID: " << m_connectedPlayers << std::endl;
    m_connectedPlayers++;
}
void GameServer::handleMessageStayAlivePing(
    std::string const& messageContent, const asio::ip::udp::endpoint& endpoint)
{
}
void GameServer::handleMessageRoundReady(
    std::string const& messageContent, const asio::ip::udp::endpoint& endpoint)
{
}

void GameServer::discard(
    std::string const& /*messageContent*/, asio::ip::udp::endpoint const& /*endpoint*/)
{
    std::cerr << "discarding message" << std::endl;
}
