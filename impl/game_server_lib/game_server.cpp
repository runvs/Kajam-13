#include "game_server.hpp"
#include <client_end_placement_data.hpp>
#include <game_properties.hpp>
#include <message.hpp>
#include <network_properties.hpp>
#include <nlohmann.hpp>
#include <object_properties.hpp>
#include <player_info.hpp>
#include <server_unit.hpp>
#include <chrono>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <thread>

GameServer::GameServer(jt::LoggerInterface& logger)
    // TODO create threadsafe logger wrapper
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

    removePlayersIfNoAlivePingReceived(elapsed);

    if (m_allPlayersReady) {
        if (!m_simulationStarted) {
            std::unique_lock<std::shared_mutex> lock { m_mutex };
            auto playerDataCopy = m_playerData;
            lock.unlock();
            startRoundSimulation(playerDataCopy);
        }
    }
}

void GameServer::removePlayersIfNoAlivePingReceived(float elapsed)
{
    std::unique_lock<std::shared_mutex> const lock { m_mutex };
    for (auto it = m_playerData.begin(); it != m_playerData.end();) {
        // think about adding a mutex here

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
    Message m = nlohmann::json::parse(messageContent);
    auto const playerId = m.playerId;
    std::unique_lock<std::shared_mutex> lock { m_mutex };
    if (m_playerData.count(playerId) == 1) {
        auto const& expectedEndpoint = m_playerData[playerId].endpoint;
        if (expectedEndpoint.address() != endpoint.address()
            || expectedEndpoint.port() != endpoint.port()) {
            m_logger.warning(
                "playerId " + std::to_string(playerId) + " does not match registered endpoint",
                { "network", "GameServer" });
            // Discard message silently
            if (m.type != MessageType::InitialPing) {
                return;
            }
        }
    }

    if (m.type != MessageType::InitialPing) {
        if (m_playerData.count(playerId) == 0) {
            m_logger.warning("message received for playerId " + std::to_string(playerId)
                    + " which is not in list of players '" + messageContent + "'",
                { "network", "GameServer" });
            return;
        }
    }

    lock.unlock();
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

    std::unique_lock<std::shared_mutex> lock { m_mutex };
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

    m_playerData[newPlayerId].timeSinceLastPing = 0.0f;
    m_playerData[newPlayerId].endpoint = endpoint;
    lock.unlock();

    Message ret;
    ret.type = MessageType::PlayerIdResponse;
    ret.playerId = newPlayerId;
    m_connection.sendMessage(ret, endpoint);
}

void GameServer::handleMessageStayAlivePing(
    std::string const& messageContent, const asio::ip::udp::endpoint& endpoint)
{
    Message const m = nlohmann::json::parse(messageContent);
    std::unique_lock<std::shared_mutex> const lock { m_mutex };
    m_playerData[m.playerId].timeSinceLastPing = 0.0f;
}

void GameServer::handleMessageRoundReady(
    std::string const& messageContent, const asio::ip::udp::endpoint& endpoint)
{
    Message const m = nlohmann::json::parse(messageContent);

    std::unique_lock<std::shared_mutex> lock { m_mutex };
    auto const playerId = m.playerId;

    m_playerData[playerId].roundReady = true;
    m_playerData[playerId].roundEndPlacementData = nlohmann::json::parse(m.data);

    bool allReady = true;
    for (auto const& kvp : m_playerData) {
        if (!kvp.second.roundReady) {
            allReady = false;
            break;
        }
    }
    lock.unlock();

    if (allReady) {
        m_allPlayersReady.store(true);
    }
}

void GameServer::discard(
    std::string const& messageContent, asio::ip::udp::endpoint const& /*endpoint*/)
{
    m_logger.warning("discard message '" + messageContent + "'", { "network", "GameServer" });
}

void GameServer::startRoundSimulation(std::map<int, PlayerInfo> const& playerData)
{
    m_logger.info("start round simulation", { "network", "GameServer" });
    m_simulationStarted.store(true);

    // TODO think about moving this into separate thread
    // TODO move this into a separate class in any case!

    // TODO make this work for more than one unit
    // TODO move number of steps to GameProperties
}
