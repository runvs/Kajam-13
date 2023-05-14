#ifndef JAMTEMPLATE_SERVER_CONNECTION_HPP
#define JAMTEMPLATE_SERVER_CONNECTION_HPP

#include "asio/ip/tcp.hpp"
#include "client_network_connection.hpp"
#include "log/logger_interface.hpp"
#include "object_properties.hpp"
#include "vector.hpp"
#include <client_end_placement_data.hpp>
#include <game_object.hpp>

class ServerConnection : public jt::GameObject {
public:
    // TODO this class will need mutexes
    explicit ServerConnection(jt::LoggerInterface& logger);
    ~ServerConnection();
    void setConnection(std::shared_ptr<ClientNetworkConnection> connection);
    std::shared_ptr<ClientNetworkConnection> getConnection();

    void readyRound(ClientEndPlacementData const& data);

    bool isRoundDataReady() const;
    std::vector<std::vector<ObjectProperties>> getRoundData();

private:
    jt::LoggerInterface& m_logger;
    int m_playerId = -1;
    std::shared_ptr<ClientNetworkConnection> m_connection;

    float m_alivePingTimer = 0.5f;

    std::atomic_bool m_dataReady { false };
    std::mutex m_dataMutex;
    std::vector<std::vector<ObjectProperties>> m_properties;

    void doUpdate(float const elapsed) override;

    void handleMessage(std::string const& messageContent, asio::ip::tcp::endpoint const& endpoint);
    void handleMessagePlayerIdResponse(std::string const& messageContent);
    void handleMessageSimulationResult(std::string const& messageContent);
    void discard(std::string const& messageContent);
};

#endif // JAMTEMPLATE_SERVER_CONNECTION_HPP
