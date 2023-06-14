#ifndef JAMTEMPLATE_INTERNAL_STATE_MANAGER_HPP
#define JAMTEMPLATE_INTERNAL_STATE_MANAGER_HPP

#include "internal_state_interface.hpp"
#include <functional>
#include <map>
#include <memory>
#include <utility>

enum class InternalState {
    WaitForAllPlayers,
    SelectStartingUnits,
    PlaceUnits,
    WaitForSimulationResults,
    Playback,
    EndWin,
    EndLose
};

class InternalStateManager {
public:
    InternalStateManager();
    void switchToState(InternalState newState, StateGame& stateGame);

    std::shared_ptr<InternalStateInterface> getActiveState();
    InternalState getActiveStateE() const;

private:
    std::shared_ptr<InternalStateInterface> m_activeStatePtr { nullptr };
    InternalState m_activeState;

    std::map<InternalState, std::shared_ptr<InternalStateInterface>> m_states;
    std::map<std::pair<InternalState, InternalState>, std::function<void(StateGame&)>>
        m_transitions;
};

#endif // JAMTEMPLATE_INTERNAL_STATE_MANAGER_HPP
