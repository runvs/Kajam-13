#include "internal_state_manager.hpp"
#include "math_helper.hpp"
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <internal_state/end_lose.hpp>
#include <internal_state/end_win.hpp>
#include <internal_state/place_units.hpp>
#include <internal_state/playback.hpp>
#include <internal_state/select_starting_units.hpp>
#include <internal_state/wait_for_all_players.hpp>
#include <internal_state/wait_for_simulation_results.hpp>
#include <state_game.hpp>
#include <unit_placement/placed_unit.hpp>
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
              auto const playerId = state.getServerConnection()->getPlayerId();

              auto const playerIdDispatcher = std::make_shared<PlayerIdDispatcher>(playerId);

              auto const unitInfo = std::make_shared<UnitInfoCollection>(
                  state.getGame()->logger(), state.getServerConnection()->getUnitInfo());

              state.setPlayerIdDispatcher(playerIdDispatcher);
              state.setUnitInfo(unitInfo);
              state.setPlacementManager(std::make_shared<PlacementManager>(
                  state.getTerrain(), playerId, playerIdDispatcher, unitInfo));

              state.setStartingUnits(state.getServerConnection()->getStartingUnits());

              state.getPlacementManager()->setActive(false);
          };

    m_transitions[std::make_pair(InternalState::SelectStartingUnits, InternalState::PlaceUnits)]
        = [](StateGame& state) {
              state.getPlacementManager()->setRound(state.getRound());
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

              for (auto& u : *state.getUnits()) {
                  auto unit = u.lock();
                  if (unit) {
                      unit->setHighlight(false);
                  }
              }
              for (auto& u : *state.getPlacementManager()->getPlacedUnits()) {
                  auto unit = u.lock();
                  if (unit) {
                      unit->setHighlight(false);
                  }
              }
          };

    m_transitions[std::make_pair(InternalState::WaitForSimulationResults, InternalState::Playback)]
        = [](StateGame& state) { state.startPlayback(); };

    m_transitions[std::make_pair(InternalState::Playback, InternalState::PlaceUnits)]
        = [numberOfLostRounds = 0, lastPlayerHP = GP::InitialPlayerHP(),
              lastOtherPlayerHP = GP::InitialPlayerHP()](StateGame& state) mutable {
              state.resetSimulation();

              state.getPlacementManager()->setRound(state.getRound());
              state.getPlacementManager()->setActive(true);
              auto const fundsForNextRound = jt::MathHelper::clamp(
                  50 + 50 * state.getRound() - state.getPlacementManager()->getCreditDebt(), 0,
                  GP::IncomePerRoundMax());
              state.getPlacementManager()->addFunds(fundsForNextRound);

              // check if additional funds are due
              auto const& playerHP = state.getPlayerHP();
              auto const playerId = state.getServerConnection()->getPlayerId();
              auto const otherPlayerId = playerId == 0 ? 1 : 0;
              if ((lastPlayerHP != playerHP.at(playerId))
                  && (lastOtherPlayerHP == playerHP.at(otherPlayerId))) {
                  numberOfLostRounds++;
                  if (numberOfLostRounds >= GP::RequiredLostRoundsForLoserBonus()) {
                      state.getPlacementManager()->addFunds(GP::IncomeLoserBonus());
                  }
              } else {
                  numberOfLostRounds = 0;
              }
              lastPlayerHP = playerHP.at(playerId);
              lastOtherPlayerHP = playerHP.at(otherPlayerId);

              state.getPlacementManager()->resetCreditDebt();

              state.getTerrainRenderer()->setDrawGrid(true);

              for (auto& c : *state.getCritters()) {
                  c.lock()->resetForNewRound();
              }
              for (auto& u : *state.getUnits()) {
                  u.lock()->resetForNewRound();
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
