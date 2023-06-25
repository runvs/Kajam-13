#include "simulation_result_message_sender.hpp"
#include <message.hpp>
#include <nlohmann.hpp>

SimulationResultMessageSender::SimulationResultMessageSender(ServerNetworkConnection& connection)
    : m_connection { connection }
{
}

void SimulationResultMessageSender::sendSimulationResults(
    SimulationResultDataForAllFrames const& data)
{
    Message m;
    m.type = MessageType::SimulationResult;
    nlohmann::json j = data;
    m.data = j.dump();

    m_connection.sendMessageToAll(m);
}
