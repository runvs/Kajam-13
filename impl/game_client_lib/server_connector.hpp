#ifndef JAMTEMPLATE_SERVER_CONNECTOR_HPP
#define JAMTEMPLATE_SERVER_CONNECTOR_HPP

#include "client_network_connection.hpp"
#include "network_properties.hpp"
#include <game_object.hpp>

class ServerConnector : public jt::GameObject {
public:
    std::shared_ptr<ClientNetworkConnection> getConnection() const;

private:
    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;

    mutable std::shared_ptr<ClientNetworkConnection> m_connection { nullptr };
    mutable std::string m_ip { "0:0:0:0:0:0:0:1" };
    mutable int m_serverPort { NetworkProperties::DefaultServerPort() };
    mutable int m_clientPort { NetworkProperties::DefaultClientPort() };
};

#endif // JAMTEMPLATE_SERVER_CONNECTOR_HPP
