#include "simulation_result_message_sender.hpp"
#include "object_properties.hpp"
#include "simulation_result_data.hpp"
#include <iostream>

SimulationResultMessageSender::SimulationResultMessageSender(ServerNetworkConnection& connection)
    : m_connection { connection }
{
}

void SimulationResultMessageSender::sendSimulationResults(
    std::vector<std::vector<ObjectProperties>> const& props)
{
    for (auto& p : props) {
        SimulationResultData data;
        data.m_unitPropertiesForOneFrame = p;
        Message m;
        m.type = MessageType::SimulationResult;
        nlohmann::json j = data;
        m.data = j.dump();

        m_connection.sendMessageToAll(m);
    }
}
