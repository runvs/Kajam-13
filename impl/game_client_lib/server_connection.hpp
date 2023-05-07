#ifndef JAMTEMPLATE_SERVER_CONNECTION_HPP
#define JAMTEMPLATE_SERVER_CONNECTION_HPP

#include "asio/ip/udp.hpp"
#include "client_network_connection.hpp"
#include "log/logger_interface.hpp"
#include "vector.hpp"
#include <client_end_placement_data.hpp>
#include <game_object.hpp>

class ServerConnection : public jt::GameObject {
public:
    explicit ServerConnection(jt::LoggerInterface& logger);
    ~ServerConnection();
    void setConnection(std::shared_ptr<ClientNetworkConnection> connection);
    std::shared_ptr<ClientNetworkConnection> getConnection();

    void readyRound(ClientEndPlacementData const& data);

private:
    jt::LoggerInterface& m_logger;
    int m_playerId = -1;
    std::shared_ptr<ClientNetworkConnection> m_connection;

    float m_alivePingTimer = 0.5f;

    void doUpdate(float const elapsed) override;

    void handleMessage(std::string const& messageContent, asio::ip::udp::endpoint const& endpoint);
    void handleMessagePlayerIdResponse(std::string const& messageContent);
    void discard(std::string const& messageContent);
};

#endif // JAMTEMPLATE_SERVER_CONNECTION_HPP
