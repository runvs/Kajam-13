#include "simulation_result_message_sender.hpp"
#include "object_properties.hpp"
#include "simulation_result_data.hpp"
#include <iostream>

SimulationResultMessageSender::SimulationResultMessageSender(ServerNetworkConnection& connection)
    : m_connection { connection }
{
}

void SimulationResultMessageSender::sendSimulationResults(
    SimulationResultDataForAllFrames const& data)
{
    for (auto const& currentFrameData : data.allFrames) {
        Message m;
        m.type = MessageType::SimulationResult;
        nlohmann::json j = currentFrameData;
        m.data = j.dump();

        m_connection.sendMessageToAll(m);
    }
}
