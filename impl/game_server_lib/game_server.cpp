#include "game_server.hpp"
#include "compression/compressor_interface.hpp"
#include "game_simulation.hpp"
#include "simulation_result_message_sender.hpp"
#include <client_end_placement_data.hpp>
#include <message.hpp>
#include <network_properties.hpp>
#include <nlohmann.hpp>
#include <player_info.hpp>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <string>

GameServer::GameServer(jt::LoggerInterface& logger, CompressorInterface& compressor)
    // TODO create thread safe logger wrapper
    : m_logger { logger }
    , m_compressor { compressor }
    , m_connection { m_compressor, logger }
{
    m_connection.setHandleIncomingMessageCallback(
        [this](auto const& messageContent, auto endpoint) {
            handleMessage(messageContent, endpoint);
        });
}

void GameServer::update(float elapsed)
{
    m_connection.update();

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
    // TODO not needed when using TCP
    std::unique_lock<std::shared_mutex> const lock { m_mutex };
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
    const std::string& messageContent, const asio::ip::tcp::endpoint& endpoint)
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
    std::string const& /*messageContent*/, asio::ip::tcp::endpoint const& endpoint)
{
    m_logger.info("initial ping received from " + endpoint.address().to_string() + ":"
            + std::to_string(endpoint.port()),
        { "network", "GameServer" });

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
    m_connection.sendMessageToOne(ret, endpoint);
}

void GameServer::handleMessageStayAlivePing(
    std::string const& messageContent, const asio::ip::tcp::endpoint& /*endpoint*/)
{
    // TODO remove as for TCP this is not needed
    Message const m = nlohmann::json::parse(messageContent);
    std::unique_lock<std::shared_mutex> const lock { m_mutex };
    m_playerData[m.playerId].timeSinceLastPing = 0.0f;
}

void GameServer::handleMessageRoundReady(
    std::string const& messageContent, const asio::ip::tcp::endpoint& endpoint)
{
    m_logger.info("Round Ready received from " + endpoint.address().to_string() + ":"
            + std::to_string(endpoint.port()),
        { "network", "GameServer" });

    Message const m = nlohmann::json::parse(messageContent);

    auto const playerId = m.playerId;
    std::unique_lock<std::shared_mutex> lock { m_mutex };
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
    std::string const& messageContent, asio::ip::tcp::endpoint const& /*endpoint*/)
{
    m_logger.warning("discard message '" + messageContent + "'", { "network", "GameServer" });
}

void GameServer::startRoundSimulation(std::map<int, PlayerInfo> const& playerData)
{
    m_logger.info("start round simulation", { "network", "GameServer" });
    m_simulationStarted.store(true);
    SimulationResultMessageSender sender { m_connection };
    GameSimulation gs { m_logger };
    gs.updateSimulationForNewRound(playerData);
    gs.performSimulation(sender);
    // TODO think about moving this into separate thread
}
