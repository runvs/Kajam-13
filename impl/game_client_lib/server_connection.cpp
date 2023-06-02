
#include "server_connection.hpp"
#include "game_properties.hpp"
#include "message.hpp"
#include "network_properties.hpp"
#include "object_properties.hpp"
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

ServerConnection::ServerConnection(jt::LoggerInterface& logger)
    : m_logger { logger }
{
}

ServerConnection::~ServerConnection()
{
    if (m_connection) {
        m_connection->setHandleIncomingMessageCallback(nullptr);
    }
}

void ServerConnection::setConnection(std::shared_ptr<ClientNetworkConnection> connection)
{
    if (!connection) {
        m_logger.fatal(
            "ServerConnection SetConnection received nullptr", { "network", "ServerConnection" });
        throw std::invalid_argument { "ServerConnection SetConnection received nullptr" };
    }
    m_connection = connection;
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
    m.playerId = m_playerId;
    nlohmann::json j = data;
    m.data = j.dump();
    m_connection->sendMessage(m);
}

void ServerConnection::handleMessage(
    std::string const& messageContent, asio::ip::tcp::endpoint const& endpoint)
{
    m_logger.verbose("handleMessage", { "network", "ServerConnection" });
    nlohmann::json j = nlohmann::json::parse(messageContent);
    Message m = j;

    if (m.type == MessageType::PlayerIdResponse) {
        handleMessagePlayerIdResponse(messageContent);
    } else if (m.type == MessageType::AllPlayersConnected) {
        handleMessageAllPlayersConnected();
    } else if (m.type == MessageType::SimulationResult) {
        handleMessageSimulationResult(messageContent);
    } else {
        discard(messageContent);
    }
}

void ServerConnection::handleMessagePlayerIdResponse(std::string const& messageContent)
{

    nlohmann::json j = nlohmann::json::parse(messageContent);
    Message const m = j;

    if (m_playerId != -1) {
        m_logger.error("Received Player Id: " + std::to_string(m.playerId)
                + ", although there was already a Player Id assigned: "
                + std::to_string(m_playerId),
            { "network", "ServerConnection" });
        return;
    }

    m_playerId = m.playerId;
    m_logger.info(
        "Received Player Id: " + std::to_string(m_playerId), { "network", "ServerConnection" });
}
void ServerConnection::handleMessageAllPlayersConnected() { m_allPlayersConnected.store(true); }

void ServerConnection::discard(std::string const& messageContent)
{
    m_logger.warning("discard message '" + messageContent + "'", { "network", "ServerConnection" });
}

void ServerConnection::handleMessageSimulationResult(std::string const& messageContent)
{
    nlohmann::json j = nlohmann::json::parse(messageContent);
    Message const m = j;
    nlohmann::json j_data = nlohmann::json::parse(m.data);
    std::vector<std::vector<ObjectProperties>> props;
    j_data.get_to(props);
    std::unique_lock<std::mutex> lock { m_dataMutex };
    m_properties.insert(m_properties.end(), props.begin(), props.end());
    m_logger.debug("received tick " + std::to_string(m_properties.size()) + " / "
        + std::to_string(GP::NumberOfStepsPerRound()));
    if (m_properties.size() == GP::NumberOfStepsPerRound()) {
        m_logger.info("received all simulation results");
        m_dataReady = true;
    }
    lock.unlock();
}
bool ServerConnection::isRoundDataReady() const { return m_dataReady; }

std::vector<std::vector<ObjectProperties>> ServerConnection::getRoundData()
{
    std::unique_lock<std::mutex> lock { m_dataMutex };
    m_dataReady = false;
    auto const propertiesCopy = m_properties;
    m_properties.clear();
    return propertiesCopy;
}
int ServerConnection::getPlayerId() const { return m_playerId; }
bool ServerConnection::areAllPlayersConnected() const { return m_allPlayersConnected; }
