#ifndef JAMTEMPLATE_SERVER_CONNECTION_HPP
#define JAMTEMPLATE_SERVER_CONNECTION_HPP

#include "internal_state/select_starting_units.hpp"
#include "network_data/select_unit_info.hpp"
#include <asio/ip/tcp.hpp>
#include <client_network_connection.hpp>
#include <client_placement_data.hpp>
#include <game_object.hpp>
#include <log/logger_interface.hpp>
#include <nlohmann.hpp>
#include <object_properties.hpp>
#include <simulation_result_data.hpp>
#include <unit_info_collection.hpp>
#include <upgrade_unit_data.hpp>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class ServerConnection : public jt::GameObject {
public:
    using ServerInfoCallback = std::function<void(nlohmann::json const& j)>;

    explicit ServerConnection(jt::LoggerInterface& logger);
    ~ServerConnection();
    void setConnection(std::shared_ptr<ClientNetworkConnection> connection);
    std::shared_ptr<ClientNetworkConnection> getConnection();

    void setOnServerInfo(ServerInfoCallback&& cb);

    void readyRound(ClientPlacementData const& data);
    void unitUpgrade(UpgradeUnitData const& data);

    bool areAllPlayersConnected() const;
    bool isRoundDataReady() const;
    SimulationResultDataForAllFrames getRoundData();

    int getPlayerId() const;
    std::vector<UnitInfo> getUnitInfo() const;
    std::shared_ptr<SelectUnitInfoCollection> getStartingUnits() const;

private:
    jt::LoggerInterface& m_logger;
    int m_playerId = -1;
    std::shared_ptr<ClientNetworkConnection> m_connection;

    std::atomic_bool m_allPlayersConnected { false };

    std::atomic_bool m_dataReady { false };
    std::mutex m_dataMutex;
    SimulationResultDataForAllFrames m_simulationResults;

    ServerInfoCallback m_serverInfoCallback;
    std::vector<UnitInfo> m_unitInfo;
    std::shared_ptr<SelectUnitInfoCollection> m_startUnits { nullptr };

    void doUpdate(float const elapsed) override;

    void handleMessage(std::string const& messageContent, asio::ip::tcp::endpoint const& endpoint);
    void handleMessagePlayerIdResponse(std::string const& messageContent);
    void handleMessageSimulationResult(std::string const& messageContent);
    void handleMessageAllPlayersConnected(nlohmann::json const& j);
    void discard(std::string const& messageContent);
};

#endif // JAMTEMPLATE_SERVER_CONNECTION_HPP
