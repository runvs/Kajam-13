#ifndef JAMTEMPLATE_SIMULATION_RESULT_MESSAGE_SENDER_HPP
#define JAMTEMPLATE_SIMULATION_RESULT_MESSAGE_SENDER_HPP

#include "object_properties.hpp"
#include "server_network_connection.hpp"

class SimulationResultMessageSender {
public:
    explicit SimulationResultMessageSender(ServerNetworkConnection& connection);

    void sendSimulationResults(std::vector<std::vector<ObjectProperties>> const& props,
        std::vector<asio::ip::udp::endpoint> const& endpoints);

private:
    ServerNetworkConnection& m_connection;
};

#endif // JAMTEMPLATE_SIMULATION_RESULT_MESSAGE_SENDER_HPP
