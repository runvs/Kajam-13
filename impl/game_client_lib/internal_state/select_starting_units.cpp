#include "select_starting_units.hpp"
#include "state_game.hpp"
#include <imgui.h>

void SelectStartingUnits::update(StateGame& state, float elapsed) { }
void SelectStartingUnits::draw(StateGame& state)
{
    ImGui::Begin("Select Starting Units");
    // TODO provide more options
    if (ImGui::Button("Select Peasant")) {
        state.getUnitInfo()->unlockType("peasant");
        state.getStateManager()->switchToState(InternalState::PlaceUnits, state);
        //        transitionSelectStartingUnitsToPlaceUnits();
    }
    ImGui::End();
}
