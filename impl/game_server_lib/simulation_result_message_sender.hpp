#ifndef JAMTEMPLATE_SIMULATION_RESULT_MESSAGE_SENDER_HPP
#define JAMTEMPLATE_SIMULATION_RESULT_MESSAGE_SENDER_HPP

#include <object_properties.hpp>
#include <server_network_connection.hpp>
#include <simulation_result_data.hpp>

class SimulationResultMessageSender {
public:
    explicit SimulationResultMessageSender(ServerNetworkConnection& connection);

    void sendSimulationResults(SimulationResultDataForAllFrames const& data);

private:
    ServerNetworkConnection& m_connection;
};

#endif // JAMTEMPLATE_SIMULATION_RESULT_MESSAGE_SENDER_HPP
