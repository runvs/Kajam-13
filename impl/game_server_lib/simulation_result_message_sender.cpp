#include "simulation_result_message_sender.hpp"
#include "object_properties.hpp"
#include <iostream>

SimulationResultMessageSender::SimulationResultMessageSender(ServerNetworkConnection& connection)
    : m_connection { connection }
{
}

void SimulationResultMessageSender::sendSimulationResults(
    std::vector<std::vector<ObjectProperties>> const& props,
    std::vector<asio::ip::udp::endpoint> const& endpoints)
{

    for (auto& p : props) {
        std::vector<std::vector<ObjectProperties>> thisProps;
        thisProps.push_back(p);
        Message m;
        m.type = MessageType::SimulationResult;
        nlohmann::json j = thisProps;
        m.data = j.dump();
        // TODO Think about not sending one message per tick but combine multiple ticks in one
        // message
        // TODO this will need tcp instead of udp
        for (auto const& e : endpoints) {
            m_connection.sendMessage(m, e);
        }
        // TODO sleep seems to be necessary otherwise client will crash
        //      - could be race condition with buffer on client side?
        //      - could be server sending packets too fast for client to process them?
        std::this_thread::sleep_for(std::chrono::milliseconds { 50 });
    }
}
