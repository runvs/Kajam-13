#ifndef JAMTEMPLATE_SERVER_CONNECTOR_HPP
#define JAMTEMPLATE_SERVER_CONNECTOR_HPP

#include "client_network_connection.hpp"
#include "compression/compressor_interface.hpp"
#include "network_properties.hpp"
#include <game_object.hpp>
#include <memory>

class ServerConnector : public jt::GameObject {
public:
    explicit ServerConnector(std::shared_ptr<CompressorInterface> compressor);
    std::shared_ptr<ClientNetworkConnection> getConnection() const;

private:
    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;

    std::shared_ptr<CompressorInterface> m_compressor { nullptr };

    mutable std::shared_ptr<ClientNetworkConnection> m_connection { nullptr };
    mutable std::string m_ip { NetworkProperties::DefaultIPAddress() };
    mutable int m_serverPort { NetworkProperties::DefaultServerPort() };
    mutable int m_clientPort { NetworkProperties::DefaultClientPort() };
    mutable bool m_addBotAsPlayerZero { false };
    mutable bool m_addBotAsPlayerOne { false };
};

#endif // JAMTEMPLATE_SERVER_CONNECTOR_HPP
