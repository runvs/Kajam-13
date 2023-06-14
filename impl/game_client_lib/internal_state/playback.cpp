#include "playback.hpp"
#include <state_game.hpp>

void Playback::update(StateGame& state, float elapsed) { state.playbackSimulation(elapsed); }
void Playback::draw(StateGame& state) { state.drawArrows(); }
