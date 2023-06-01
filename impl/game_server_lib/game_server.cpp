#include "game_server.hpp"
#include "json_keys.hpp"
#include <compression/compressor_interface.hpp>
#include <game_simulation.hpp>
#include <message.hpp>
#include <network_properties.hpp>
#include <nlohmann.hpp>
#include <player_info.hpp>
#include <simulation_result_message_sender.hpp>
#include <iostream>
#include <mutex>
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
            std::unique_lock<std::mutex> lock { m_mutex };

            auto const playerDataCopy = m_playerData;
            auto botDataCopy = m_botData;
            lock.unlock();

            PerformAI(botDataCopy);

            startRoundSimulation(playerDataCopy, botDataCopy);
        }
    }
}

void GameServer::PerformAI(std::map<int, PlayerInfo>& botDataCopy) const
{
    if (!botDataCopy.empty()) {
        ObjectProperties props;
        props.ints[jk::unitID] = 0;
        props.ints[jk::playerID] = botDataCopy.begin()->first;
        props.floats[jk::positionX] = botDataCopy.begin()->first == 0 ? 50 : 200;
        props.floats[jk::positionY] = 100.0f;

        botDataCopy.begin()->second.roundEndPlacementData.m_properties.push_back(props);
    }
}

void GameServer::removePlayersIfNoAlivePingReceived(float elapsed)
{
    std::unique_lock<std::mutex> const lock { m_mutex };
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
    std::unique_lock<std::mutex> lock { m_mutex };
    // check if player id is known
    if (m_playerData.count(playerId) == 1) {
        auto const& expectedEndpoint = m_playerData[playerId].endpoint;
        // check if endpoint matches known player ID
        if (expectedEndpoint.address() != endpoint.address()
            || expectedEndpoint.port() != endpoint.port()) {
            m_logger.warning(
                "playerId " + std::to_string(playerId) + " does not match registered endpoint",
                { "network", "GameServer" });
            // Discard message silently
            if (m.type != MessageType::InitialPing && m.type != MessageType::AddBot) {
                return;
            }
        }
    }

    if (m.type != MessageType::InitialPing && m.type != MessageType::AddBot) {
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
    } else if (m.type == MessageType::AddBot) {
        handleMessageAddBot();
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

    std::unique_lock<std::mutex> lock { m_mutex };
    for (auto& kvp : m_playerData) {
        if (kvp.second.endpoint.address() == endpoint.address()
            && kvp.second.endpoint.port() == endpoint.port()) {
            m_logger.warning(
                "player address already registered, not adding the connection a second time",
                { "network", "GameServer" });
            return;
        }
    }

    if (getNumberOfConnectedPlayers() >= 2) {
        m_logger.warning("already two players connected", { "network", "GameServer" });
        return;
    }
    int const newPlayerId = getNumberOfConnectedPlayers();
    m_logger.info(
        "assigned new player ID: " + std::to_string(newPlayerId), { "network", "GameServer" });

    m_playerData[newPlayerId].timeSinceLastPing = 0.0f;
    m_playerData[newPlayerId].endpoint = endpoint;
    lock.unlock();

    {
        Message ret;
        ret.type = MessageType::PlayerIdResponse;
        ret.playerId = newPlayerId;
        m_connection.sendMessageToOne(ret, endpoint);
    }
    // inform all players that the requested number of Players is reached
    if (getNumberOfConnectedPlayers() == 2) {
        Message ret;
        ret.type = MessageType::AllPlayersConnected;
        m_connection.sendMessageToAll(ret);
    }
}

void GameServer::handleMessageAddBot()
{
    m_logger.info("Add bot received", { "network", "GameServer" });
    std::unique_lock<std::mutex> lock { m_mutex };
    if (getNumberOfConnectedPlayers() >= 2) {
        m_logger.warning("already two players connected", { "network", "GameServer" });
        return;
    }
    int const newPlayerId = getNumberOfConnectedPlayers();
    m_logger.info(
        "assign bot new player ID: " + std::to_string(newPlayerId), { "network", "GameServer" });

    m_botData[newPlayerId].timeSinceLastPing = 0.0f;
    m_botData[newPlayerId];
    lock.unlock();

    // inform all players that the requested number of Players is reached
    if (getNumberOfConnectedPlayers() == 2) {
        Message ret;
        ret.type = MessageType::AllPlayersConnected;
        m_connection.sendMessageToAll(ret);
    }
}

void GameServer::handleMessageStayAlivePing(
    std::string const& messageContent, const asio::ip::tcp::endpoint& /*endpoint*/)
{
    // TODO remove as for TCP this is not needed
    Message const m = nlohmann::json::parse(messageContent);
    std::unique_lock<std::mutex> const lock { m_mutex };
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
    std::unique_lock<std::mutex> lock { m_mutex };
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

void GameServer::startRoundSimulation(
    std::map<int, PlayerInfo> const& playerData, std::map<int, PlayerInfo> const& botData)
{
    std::map<int, PlayerInfo> combinedData;
    // TODO check that ids are unique
    combinedData.insert(playerData.cbegin(), playerData.cend());
    combinedData.insert(botData.cbegin(), botData.cend());

    m_logger.info("start round simulation", { "network", "GameServer" });
    // TODO think about moving this into separate thread
    m_simulationStarted.store(true);
    SimulationResultMessageSender sender { m_connection };
    GameSimulation gs { m_logger };
    gs.updateSimulationForNewRound(combinedData);
    gs.performSimulation(sender);
}
int GameServer::getNumberOfConnectedPlayers() const
{
    return m_playerData.size() + m_botData.size();
}
