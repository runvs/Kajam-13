#include "end_lose.hpp"
#include <game_interface.hpp>
#include <state_game.hpp>
#include <state_menu.hpp>
#include <imgui.h>

void EndLose::update(StateGame& state, float elapsed) { }
void EndLose::draw(StateGame& state)
{
    ImGui::Begin("Game Over");
    ImGui::Text("Lost");

    if (ImGui::Button("Back to menu")) {
        state.getGame()->stateManager().switchState(std::make_shared<StateMenu>());
    }
    ImGui::End();
}
