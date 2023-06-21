#include "internal_state_manager.hpp"
#include "end_lose.hpp"
#include "end_win.hpp"
#include "place_units.hpp"
#include "playback.hpp"
#include "select_starting_units.hpp"
#include "state_game.hpp"
#include "wait_for_all_players.hpp"
#include "wait_for_simulation_results.hpp"
#include <memory>

InternalStateManager::InternalStateManager()
{
    m_states[InternalState::WaitForAllPlayers] = std::make_shared<WaitForAllPlayers>();
    m_states[InternalState::SelectStartingUnits] = std::make_shared<SelectStartingUnits>();
    m_states[InternalState::PlaceUnits] = std::make_shared<PlaceUnits>();
    m_states[InternalState::WaitForSimulationResults]
        = std::make_shared<WaitForSimulationResults>();
    m_states[InternalState::Playback] = std::make_shared<Playback>();
    m_states[InternalState::EndWin] = std::make_shared<EndWin>();
    m_states[InternalState::EndLose] = std::make_shared<EndLose>();

    m_transitions[std::make_pair(
        InternalState::WaitForAllPlayers, InternalState::SelectStartingUnits)]
        = [](StateGame& state) {
              state.transitionWaitForPlayersToSelectStartingUnits();
              state.getPlacementManager()->setActive(false);
          };

    m_transitions[std::make_pair(InternalState::SelectStartingUnits, InternalState::PlaceUnits)]
        = [](StateGame& state) {
              state.getPlacementManager()->setActive(true);
              state.getTerrainRenderer()->setDrawGrid(true);
          };

    m_transitions[std::make_pair(
        InternalState::PlaceUnits, InternalState::WaitForSimulationResults)]
        = [](StateGame& state) {
              ClientPlacementData clientEndPlacementData;
              clientEndPlacementData.m_units
                  = state.getPlacementManager()->getPlacedUnitDataForRoundStart();
              state.getServerConnection()->readyRound(clientEndPlacementData);
              state.getPlacementManager()->clearPlacedUnits();

              state.getPlacementManager()->setActive(false);
              state.getTerrainRenderer()->setDrawGrid(false);
          };

    m_transitions[std::make_pair(InternalState::WaitForSimulationResults, InternalState::Playback)]
        = [](StateGame& state) { state.transitionWaitForSimulationResultsToPlayback(); };

    m_transitions[std::make_pair(InternalState::Playback, InternalState::PlaceUnits)]
        = [](StateGame& state) {
              state.transitionPlaybackToPlaceUnits();
              state.getPlacementManager()->setActive(true);
              state.getPlacementManager()->addFunds(100 + 50 * state.getRound());

              state.getTerrainRenderer()->setDrawGrid(true);

              for (auto& b : *state.getBirds()) {
                  b.lock()->resetOnRoundStart();
              }
              for (auto& b : *state.getUnits()) {
                  b.lock()->resetForNewRound();
              }
          };

    m_activeState = InternalState::WaitForAllPlayers;
    m_activeStatePtr = m_states.at(InternalState::WaitForAllPlayers);
}

std::shared_ptr<InternalStateInterface> InternalStateManager::getActiveState()
{
    return m_activeStatePtr;
}

void InternalStateManager::switchToState(InternalState newState, StateGame& stateGame)
{
    auto const transitionPair = std::make_pair(m_activeState, newState);
    if (m_transitions.count(transitionPair) == 1) {
        auto& transition = m_transitions.at(transitionPair);
        transition(stateGame);
    }
    m_activeState = newState;
    m_activeStatePtr = m_states.at(newState);
}

InternalState InternalStateManager::getActiveStateE() const { return m_activeState; }
