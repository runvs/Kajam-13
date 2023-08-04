#ifndef JAMTEMPLATE_WAIT_FOR_SIMULATION_RESULTS_HPP
#define JAMTEMPLATE_WAIT_FOR_SIMULATION_RESULTS_HPP

#include <internal_state/internal_state_interface.hpp>

class WaitForSimulationResults : public InternalStateInterface {
public:
    void create(StateGame& state) override;
    void update(StateGame& state, float elapsed) override;
    void draw(StateGame& state) override;
};

#endif // JAMTEMPLATE_WAIT_FOR_SIMULATION_RESULTS_HPP
