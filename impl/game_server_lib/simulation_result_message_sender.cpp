#include "simulation_result_message_sender.hpp"
#include "object_properties.hpp"
#include <iostream>

SimulationResultMessageSender::SimulationResultMessageSender(ServerNetworkConnection& connection)
    : m_connection { connection }
{
}

void SimulationResultMessageSender::sendSimulationResults(
    std::vector<std::vector<ObjectProperties>> const& props,
    std::vector<asio::ip::tcp::endpoint> const& endpoints)
{
    for (auto& p : props) {
        std::vector<std::vector<ObjectProperties>> thisProps;
        thisProps.push_back(p);
        Message m;
        m.type = MessageType::SimulationResult;
        nlohmann::json j = thisProps;
        m.data = j.dump();

        m_connection.sendMessageToAll(m);
    }
}
