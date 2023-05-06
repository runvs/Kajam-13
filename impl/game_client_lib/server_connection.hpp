#ifndef JAMTEMPLATE_SERVER_CONNECTION_HPP
#define JAMTEMPLATE_SERVER_CONNECTION_HPP

#include "client_network_connection.hpp"
#include "vector.hpp"
#include <client_end_placement_data.hpp>
#include <game_object.hpp>

class ServerConnection : public jt::GameObject {
public:
    ~ServerConnection();
    void setConnection(std::shared_ptr<ClientNetworkConnection> connection);
    std::shared_ptr<ClientNetworkConnection> getConnection();

    void readyRound(ClientEndPlacementData const& data);

private:
    int m_playerId = -1;
    std::shared_ptr<ClientNetworkConnection> m_connection;

    float m_alivePingTimer = 0.5f;

    void doUpdate(float const elapsed) override;
};

#endif // JAMTEMPLATE_SERVER_CONNECTION_HPP
