#ifndef JAMTEMPLATE_SERVER_CONNECTION_HPP
#define JAMTEMPLATE_SERVER_CONNECTION_HPP

#include "simulation_result_data.hpp"
#include "vector.hpp"
#include <asio/ip/tcp.hpp>
#include <client_network_connection.hpp>
#include <client_placement_data.hpp>
#include <game_object.hpp>
#include <log/logger_interface.hpp>
#include <object_properties.hpp>
#include <unit_info_collection.hpp>

class ServerConnection : public jt::GameObject {
public:
    explicit ServerConnection(jt::LoggerInterface& logger);
    ~ServerConnection();
    void setConnection(std::shared_ptr<ClientNetworkConnection> connection);
    std::shared_ptr<ClientNetworkConnection> getConnection();

    void readyRound(ClientPlacementData const& data);

    bool areAllPlayersConnected() const;
    bool isRoundDataReady() const;
    SimulationResultDataForAllFrames getRoundData();

    int getPlayerId() const;
    std::vector<UnitInfo> getUnitInfo() const;

private:
    jt::LoggerInterface& m_logger;
    int m_playerId = -1;
    std::shared_ptr<ClientNetworkConnection> m_connection;

    float m_alivePingTimer = 0.5f;

    std::atomic_bool m_allPlayersConnected { false };

    std::atomic_bool m_dataReady { false };
    std::mutex m_dataMutex;
    SimulationResultDataForAllFrames m_simulationResults;

    std::vector<UnitInfo> m_unitInfo;

    void doUpdate(float const elapsed) override;

    void handleMessage(std::string const& messageContent, asio::ip::tcp::endpoint const& endpoint);
    void handleMessagePlayerIdResponse(std::string const& messageContent);
    void handleMessageSimulationResult(std::string const& messageContent);
    void handleMessageAllPlayersConnected();
    void discard(std::string const& messageContent);
};

#endif // JAMTEMPLATE_SERVER_CONNECTION_HPP
