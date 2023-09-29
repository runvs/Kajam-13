#include "wait_for_all_players.hpp"
#include <internal_state/internal_state_manager.hpp>
#include <state_game.hpp>

void WaitForAllPlayers::create(StateGame& /*state*/) { }

void WaitForAllPlayers::update(StateGame& state, float /*elapsed*/)
{
    if (state.getServerConnection()->areAllPlayersConnected()) {

        state.getStateManager()->switchToState(InternalState::SelectStartingUnits, state);
    }
}

void WaitForAllPlayers::draw(StateGame& /*state*/) { }
