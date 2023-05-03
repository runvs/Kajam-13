
#include "server_connection.h"
#include "network_properties.hpp"

void ServerConnection::setConnection(std::shared_ptr<ClientNetworkConnection> connection)
{
    m_connection = connection;
}

std::shared_ptr<ClientNetworkConnection> ServerConnection::getConnection() { return m_connection; }

void ServerConnection::doUpdate(const float elapsed)
{
    m_alivePingTimer -= elapsed;
    if (m_alivePingTimer <= 0) {
        m_alivePingTimer += NetworkProperties::AlivePingTimer();
        m_connection->sendAlivePing();
    }
}
