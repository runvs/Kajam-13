#include "playback.hpp"
#include <internal_state/common_functions.hpp>
#include <state_game.hpp>

void Playback::update(StateGame& state, float elapsed)
{
    state.playbackSimulation(elapsed);
    CommonFunctions::updateCritters(state);
}

void Playback::draw(StateGame& state)
{
    state.drawArrows();
    CommonFunctions::drawUnitTooltips(state);
}
