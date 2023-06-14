#include "place_units.hpp"
#include "internal_state_manager.hpp"
#include "state_game.hpp"
#include <imgui.h>

void PlaceUnits::update(StateGame& state, float elapsed) { }
void PlaceUnits::draw(StateGame& state)
{

    state.getPlacementManager()->draw();

    ImGui::Begin("End Placement");
    if (ImGui::Button("End Placement")) {
        state.getStateManager()->switchToState(InternalState::WaitForSimulationResults, state);
    }
    ImGui::End();
}
