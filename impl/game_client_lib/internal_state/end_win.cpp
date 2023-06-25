#include "end_win.hpp"
#include <game_interface.hpp>
#include <state_game.hpp>
#include <state_menu.hpp>
#include <imgui.h>

void EndWin::update(StateGame& /*state*/, float /*elapsed*/) { }
void EndWin::draw(StateGame& state)
{
    if (!m_imageVictory) {
        m_imageVictory = std::make_shared<jt::Sprite>("assets/images/menus/victory.png",
            jt::Recti { 0, 0, 768, 512 }, state.getGame()->gfx().textureManager());
    }
    if (!m_imageBackToMenu) {
        m_imageBackToMenu = std::make_shared<jt::Sprite>("assets/images/menus/back_to_menu.png",
            jt::Recti { 0, 0, 483, 64 }, state.getGame()->gfx().textureManager());
    }

    ImGuiWindowFlags window_flags { ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoScrollWithMouse };
    ImGui::SetNextWindowPos(
        { GP::GetWindowSize().x / 2 - 206, GP::GetWindowSize().y / 2 - 160 }, ImGuiCond_Always);
    ImGui::SetNextWindowSize({ 412, 0 }, ImGuiCond_Always);
    ImGui::Begin("Game Over", nullptr, window_flags);

    ImGui::Image(m_imageVictory->getSFSprite().getTexture()->getNativeHandle(), { 408.0f, 272.0f });
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Victory");
        ImGui::EndTooltip();
    }

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 196) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(212, 180, 134, 120));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 0));
    if (ImGui::ImageButton(m_imageBackToMenu->getSFSprite().getTexture()->getNativeHandle(),
            { 196, 26 }, { 0, 0 }, { 1, 1 }, 0)) {
        state.getGame()->stateManager().switchState(std::make_shared<StateMenu>());
    }
    ImGui::PopStyleColor(3);
    ImGui::End();

    // TODO add some particle effects
}
