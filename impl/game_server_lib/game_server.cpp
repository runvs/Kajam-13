#include "game_server.hpp"
#include "system_helper.hpp"
#include <compression/compressor_interface.hpp>
#include <game_properties.hpp>
#include <game_simulation.hpp>
#include <json_keys.hpp>
#include <message.hpp>
#include <network_properties.hpp>
#include <nlohmann.hpp>
#include <player_info.hpp>
#include <simulation_result_message_sender.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

GameServer::GameServer(jt::LoggerInterface& logger, CompressorInterface& compressor)
    : m_logger { logger }
    , m_compressor { compressor }
    , m_connection { m_compressor, logger }
    , m_unitInfos { m_logger }
    , m_gameSimulation { std::make_unique<GameSimulation>(m_logger, m_unitInfos) }
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

            performAI(botDataCopy);

            startRoundSimulation(playerDataCopy, botDataCopy);

            for (auto& p : m_playerData) {
                m_logger.info(
                    "Round Simulation done. Waiting for next round", { "network", "GameServer" });
                p.second.roundReady = false;
            }
            m_simulationStarted = false;
            m_allPlayersReady = false;
        }
    }
}

void GameServer::performAI(std::map<int, PlayerInfo>& botDataCopy) const
{
    if (!botDataCopy.empty()) {
        ObjectProperties props;
        props.ints[jk::unitID] = m_round;
        props.ints[jk::playerID] = botDataCopy.begin()->first;
        props.floats[jk::positionX]
            = static_cast<float>(botDataCopy.begin()->first == 0 ? 48 : GP::GetScreenSize().x - 47);
        props.floats[jk::positionY] = terrainChunkSizeInPixel * 10 + 32 * m_round;
        std::vector<std::string> const possibleUnits = m_unitInfos.getTypes();
        auto const unitType
            = *jt::SystemHelper::select_randomly(possibleUnits.cbegin(), possibleUnits.cend());
        ;
        props.floats[jk::offsetX] = 0.0f;
        props.floats[jk::offsetY] = 0.0f;
        props.strings[jk::unitType] = unitType;

        m_gameSimulation->addUnit(props);
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
    // also clear player data if socket is closed or no longer in list of sockets
    for (auto it = m_playerData.begin(); it != m_playerData.end();) {
        if (!m_connection.isSocketOpenFor(it->second.endpoint)) {
            std::stringstream ss_log;
            ss_log << "erase endpoint '" << it->second.endpoint.address()
                   << "' because of closed port for player " << it->first << "\n";
            m_logger.info(ss_log.str(), { "network", "GameServer" });

            it = m_playerData.erase(it);
        } else {
            ++it;
        }
    }
    if (m_playerData.empty()) {
        resetServer();
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
        nlohmann::json j = nlohmann::json { { jk::units, m_unitInfos.getUnits() } };
        ret.data = j.dump();
        m_connection.sendMessageToOne(ret, endpoint);
    }
    // inform all players that the requested number of Players is reached
    checkForAllPlayersConnected();
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

    checkForAllPlayersConnected();
}

void GameServer::handleMessageStayAlivePing(
    std::string const& messageContent, const asio::ip::tcp::endpoint& /*endpoint*/)
{
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

    if (!m_matchHasStarted) {
        m_logger.warning("round ready data received although game has not started",
            { "network", "game_server" });
    }

    Message const m = nlohmann::json::parse(messageContent);

    auto const playerId = m.playerId;
    std::unique_lock<std::mutex> lock { m_mutex };
    m_playerData[playerId].roundReady = true;
    m_playerData[playerId].roundEndPlacementData = nlohmann::json::parse(m.data);
    for (auto const& props : m_playerData[playerId].roundEndPlacementData.m_properties) {
        // add new unity to game simulation
        m_gameSimulation->addUnit(props);
    }
    bool allReady = true;
    for (auto const& kvp : m_playerData) {
        if (!kvp.second.roundReady) {
            allReady = false;
            break;
        }
    }
    lock.unlock();

    if (allReady) {
        m_logger.info("all players ready");
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
    // merge player and bot data
    std::map<int, PlayerInfo> combinedData;
    // TODO check that ids are unique
    combinedData.insert(playerData.cbegin(), playerData.cend());
    combinedData.insert(botData.cbegin(), botData.cend());

    m_logger.info(
        "start round simulation for round " + std::to_string(m_round), { "network", "GameServer" });

    m_simulationStarted.store(true);
    SimulationResultMessageSender sender { m_connection };

    m_gameSimulation->prepareSimulationForNewRound();
    m_gameSimulation->performSimulation(sender);
    m_round++;
}
int GameServer::getNumberOfConnectedPlayers() const
{
    return static_cast<int>(m_playerData.size() + m_botData.size());
}
void GameServer::checkForAllPlayersConnected()
{
    // inform all players that the requested number of Players is reached
    if (getNumberOfConnectedPlayers() == 2) {
        Message ret;
        ret.type = MessageType::AllPlayersConnected;
        m_connection.sendMessageToAll(ret);
        m_matchHasStarted = true;
    }
}

void GameServer::resetServer()
{
    if (!m_matchHasStarted) {
        return;
    }
    m_logger.info("no more players connected, resetting server", { "GameServer" });
    m_matchHasStarted = false;
    m_round = 1;
    if (!m_botData.empty()) {
        m_botData.clear();
    }
}
