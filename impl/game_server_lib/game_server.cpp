#include "game_server.hpp"
#include "client_end_placement_data.hpp"
#include "message.hpp"
#include "network_properties.hpp"
#include "player_info.hpp"
#include <nlohmann.hpp>
#include <iostream>
#include <sstream>
#include <string>

GameServer::GameServer(jt::LoggerInterface& logger)
    : m_logger { logger }
{
    m_connection.setHandleIncomingMessageCallback(
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
            std::stringstream ss_log;
            ss_log << "erase endpoint '" << it->second.endpoint.address()
                   << "' because of missing ping for player " << it->first << "\n";
            m_logger.info(ss_log.str(), { "network", "GameServer" });

            it = m_playerData.erase(it);

        } else {
            ++it;
        }
    }
}
void GameServer::handleMessage(
    const std::string& messageContent, const asio::ip::udp::endpoint& endpoint)
{
    m_logger.debug(
        "handleMessage message content: '" + messageContent + "'", { "network", "GameServer" });
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
    m_logger.debug("initial ping received", { "network", "GameServer" });
    // TODO check if player is already registered
    // TODO check if server is already full
    m_playerData[m_connectedPlayers] = PlayerInfo {};
    m_playerData[m_connectedPlayers].timeSinceLastPing = 0.0f;
    m_playerData[m_connectedPlayers].endpoint = endpoint;

    Message ret;
    ret.type = MessageType::PlayerIdResponse;
    ret.playerId = m_connectedPlayers;
    m_connection.sendMessage(ret, endpoint);

    m_logger.info("assigned new player ID: " + std::to_string(m_connectedPlayers),
        { "network", "GameServer" });
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
    std::string const& messageContent, asio::ip::udp::endpoint const& /*endpoint*/)
{
    m_logger.warning("discard message '" + messageContent + "'", { "network", "GameServer" });
}
