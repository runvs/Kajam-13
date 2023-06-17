#include "wait_for_simulation_results.hpp"
#include "internal_state_manager.hpp"
#include "state_game.hpp"

void WaitForSimulationResults::update(StateGame& state, float /*elapsed*/)
{
    if (state.getServerConnection()->isRoundDataReady()) {
        state.getStateManager()->switchToState(InternalState::Playback, state);
    }
}

void WaitForSimulationResults::draw(StateGame& /*state*/) { }
