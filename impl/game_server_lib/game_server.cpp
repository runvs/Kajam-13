#include "game_server.hpp"
#include <compression/compressor_interface.hpp>
#include <game_properties.hpp>
#include <game_simulation.hpp>
#include <json_keys.hpp>
#include <message.hpp>
#include <network_data/unit_client_to_server_data.hpp>
#include <network_properties.hpp>
#include <nlohmann.hpp>
#include <player_info.hpp>
#include <simulation_result_message_sender.hpp>
#include <system_helper.hpp>
#include <upgrade_unit_data.hpp>
#include <fstream>
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
    m_startingUnits = std::make_unique<SelectUnitInfoCollection>();

    std::ifstream infile { "assets/units/starting_units.json" };
    *m_startingUnits = nlohmann::json::parse(infile);
    m_logger.info("Parsed starting units file");
    m_connection.setHandleIncomingMessageCallback(
        [this](auto const& messageContent, auto endpoint) {
            handleMessage(messageContent, endpoint);
        });
}

void GameServer::update(float elapsed)
{
    m_connection.update();

    removePlayerIfConnectionClosed(elapsed);

    if (m_allPlayersReady) {
        if (!m_simulationStarted) {
            std::unique_lock<std::mutex> lock { m_mutex };
            auto botDataCopy = m_botData;
            lock.unlock();

            performAI(botDataCopy);

            startRoundSimulation();

            m_logger.info(
                "Round Simulation done. Waiting for next round", { "network", "GameServer" });
            for (auto& p : m_playerData) {
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
        UnitClientToServerData unitData;
        unitData.unitID = m_round;
        unitData.playerID = botDataCopy.begin()->first;
        unitData.positionX
            = static_cast<float>(botDataCopy.begin()->first == 0 ? 48 : GP::GetScreenSize().x - 47);
        unitData.positionY = terrainChunkSizeInPixel * 5.0f + 32.0f * m_round;
        while (unitData.positionY >= ((terrainHeightInChunks - 3) * terrainChunkSizeInPixel)) {
            unitData.positionX -= terrainChunkSizeInPixel * 2;
            unitData.positionY -= terrainChunkSizeInPixel * 5 + 32 * (m_round - 10);
        }
        auto const possibleUnits = m_unitInfos.getTypes();
        auto const unitType
            = *jt::SystemHelper::select_randomly(possibleUnits.cbegin(), possibleUnits.cend());

        unitData.offsetX = 0.0f;
        unitData.offsetY = 0.0f;
        unitData.unitType = unitType;

        m_gameSimulation->addUnit(unitData);
    }
}

void GameServer::removePlayerIfConnectionClosed(float /*elapsed*/)
{
    std::unique_lock<std::mutex> const lock { m_mutex };
    // clear player data if socket is closed or no longer in list of sockets
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
    } else if (m.type == MessageType::RoundReady) {
        handleMessageRoundReady(messageContent, endpoint);
    } else if (m.type == MessageType::UnitUpgrade) {
        handleMessageUnitUpgrade(m);
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

    m_playerData[newPlayerId].endpoint = endpoint;
    lock.unlock();

    {
        Message ret;
        ret.type = MessageType::PlayerIdResponse;
        ret.playerId = newPlayerId;
        nlohmann::json j = nlohmann::json { { jk::units, m_unitInfos.getUnits() },
            { "startingUnits", *m_startingUnits } };
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

    m_botData[newPlayerId];
    lock.unlock();

    checkForAllPlayersConnected();
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
    for (auto const& props : m_playerData[playerId].roundEndPlacementData.m_units) {
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

void GameServer::handleMessageUnitUpgrade(Message const& m)
{
    m_logger.info("Unit upgrade received", { "network", "GameServer" });

    if (!m_matchHasStarted) {
        m_logger.warning("unit upgrade data received although game has not started",
            { "network", "game_server" });
    }
    if (m.type != MessageType::UnitUpgrade) {
        m_logger.warning(
            "unit upgrade called with message of incorrect type", { "network", "game_server" });
        return;
    }
    nlohmann::json j = nlohmann::json::parse(m.data);

    UpgradeUnitData data = j;
    m_gameSimulation->addUnitUpgrade(data);
}

void GameServer::discard(
    std::string const& messageContent, asio::ip::tcp::endpoint const& /*endpoint*/)
{
    m_logger.warning("discard message '" + messageContent + "'", { "network", "GameServer" });
}

void GameServer::startRoundSimulation()
{
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
        m_gameSimulation->rollNewMap();
        Message ret;
        ret.type = MessageType::AllPlayersConnected;
        nlohmann::json const j { { jk::map, m_gameSimulation->getTerrain() } };
        ret.data = j.dump();
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
    m_gameSimulation->clear();
    m_round = 1;
    if (!m_botData.empty()) {
        m_botData.clear();
    }
}
