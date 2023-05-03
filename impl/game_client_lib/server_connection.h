#ifndef JAMTEMPLATE_SERVER_CONNECTION_H
#define JAMTEMPLATE_SERVER_CONNECTION_H

#include "client_network_connection.hpp"
#include <game_object.hpp>

class ServerConnection : public jt::GameObject {
public:
    void setConnection(std::shared_ptr<ClientNetworkConnection> connection);

    std::shared_ptr<ClientNetworkConnection> getConnection();

private:

    std::shared_ptr<ClientNetworkConnection> m_connection;

    float m_alivePingTimer = 0.5f;

    void doUpdate(float const elapsed) override;

};


#endif //JAMTEMPLATE_SERVER_CONNECTION_H
