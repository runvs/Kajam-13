#ifndef JAMTEMPLATE_SIMULATION_RESULT_SENDER_INTERFACE_HPP
#define JAMTEMPLATE_SIMULATION_RESULT_SENDER_INTERFACE_HPP

#include <simulation_result_data.hpp>

class SimulationResultSenderInterface {
public:
    virtual ~SimulationResultSenderInterface() = default;
    virtual void sendSimulationResults(SimulationResultDataForAllFrames const& data) = 0;
};

#endif // JAMTEMPLATE_SIMULATION_RESULT_SENDER_INTERFACE_HPP
