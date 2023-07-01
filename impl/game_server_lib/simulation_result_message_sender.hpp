#ifndef JAMTEMPLATE_SIMULATION_RESULT_MESSAGE_SENDER_HPP
#define JAMTEMPLATE_SIMULATION_RESULT_MESSAGE_SENDER_HPP

#include <object_properties.hpp>
#include <server_network_connection.hpp>
#include <simulation_result_sender_interface.hpp>

class SimulationResultMessageSender : public SimulationResultSenderInterface {
public:
    explicit SimulationResultMessageSender(ServerNetworkConnection& connection);

    void sendSimulationResults(SimulationResultDataForAllFrames const& data) override;

private:
    ServerNetworkConnection& m_connection;
};

#endif // JAMTEMPLATE_SIMULATION_RESULT_MESSAGE_SENDER_HPP
