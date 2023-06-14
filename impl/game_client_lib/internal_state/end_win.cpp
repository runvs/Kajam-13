#include "end_win.hpp"
#include <game_interface.hpp>
#include <state_game.hpp>
#include <state_menu.hpp>
#include <imgui.h>

void EndWin::update(StateGame& state, float elapsed) { }
void EndWin::draw(StateGame& state)
{
    ImGui::Begin("Game Over");
    ImGui::Text("Game over");

    ImGui::Text("Win");

    if (ImGui::Button("Back to menu")) {
        state.getGame()->stateManager().switchState(std::make_shared<StateMenu>());
    }
    ImGui::End();
}
