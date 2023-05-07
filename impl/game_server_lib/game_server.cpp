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
        if (it->second.timeSinceLastPing >= 5.0f) {
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
    auto const playerId = m.playerId;
    if (m_playerData.count(playerId) == 1) {
        auto const& expectedEndpoint = m_playerData[playerId].endpoint;
        if (expectedEndpoint.address() != endpoint.address()
            || expectedEndpoint.port() != endpoint.port()) {
            m_logger.warning(
                "playerId does not match registered endpoint", { "network", "GameServer" });
            // Discard message silently
            if (m.type != MessageType::InitialPing) {
                return;
            }
        }
    }
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

    for (auto& kvp : m_playerData) {
        if (kvp.second.endpoint.address() == endpoint.address()
            && kvp.second.endpoint.port() == endpoint.port()) {
            m_logger.warning(
                "player address already registered, not adding the connection a second time",
                { "network", "GameServer" });
            return;
        }
    }

    if (m_playerData.size() >= 2) {
        m_logger.warning("already two players connected", { "network", "GameServer" });
        return;
    }
    int const newPlayerId = m_connectedPlayers;
    m_connectedPlayers++;
    m_logger.info(
        "assigned new player ID: " + std::to_string(newPlayerId), { "network", "GameServer" });
    m_playerData[newPlayerId] = PlayerInfo {};
    m_playerData[newPlayerId].timeSinceLastPing = 0.0f;
    m_playerData[newPlayerId].endpoint = endpoint;

    Message ret;
    ret.type = MessageType::PlayerIdResponse;
    ret.playerId = newPlayerId;
    m_connection.sendMessage(ret, endpoint);
}

void GameServer::handleMessageStayAlivePing(
    std::string const& messageContent, const asio::ip::udp::endpoint& endpoint)
{
    nlohmann::json const j = nlohmann::json::parse(messageContent);
    Message const m = j;

    m_playerData[m.playerId].timeSinceLastPing = 0.0f;
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
