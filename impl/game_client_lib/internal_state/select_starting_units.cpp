#include "select_starting_units.hpp"
#include <network_data/select_unit_info.hpp>
#include <state_game.hpp>
#include <imgui.h>
#include <string>

void SelectStartingUnits::create(StateGame& /*state*/) { }

void SelectStartingUnits::update(StateGame& /*state*/, float /*elapsed*/) { }

void SelectStartingUnits::draw(StateGame& state)
{
    ImGuiWindowFlags window_flags { ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
        | ImGuiWindowFlags_AlwaysAutoResize };
    ImGui::SetNextWindowPos({ 40, GP::GetWindowSize().y / 2 - 100 }, ImGuiCond_Always);
    ImGui::SetNextWindowSize({ 284, 100 }, ImGuiCond_Always);
    ImGui::Begin("Select Starting Units", nullptr, window_flags);

    if (ImGui::BeginTable("Startout Options", 5)) {
        int i = 0;
        for (auto const& opt : state.getStartingUnits()->m_selectUnitInfos) {
            i++;
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", opt.unit1.c_str());
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "%s", state.getUnitInfo()->getInfoForType(opt.unit1).description.c_str());
            }

            ImGui::TableNextColumn();
            ImGui::Text("%s", opt.unit2.c_str());
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(
                    "%s", state.getUnitInfo()->getInfoForType(opt.unit2).description.c_str());
            }
            ImGui::TableNextColumn();
            ImGui::Text("%i", opt.initialGold);

            ImGui::TableNextColumn();
            ImGui::Text("%s", opt.effect.c_str());
            ImGui::TableNextColumn();
            auto buttonText = "Buy##" + std::to_string(i);
            if (ImGui::Button(buttonText.c_str())) {
                state.getPlacementManager()->unlockType(opt.unit1);
                state.getPlacementManager()->unlockType(opt.unit2);
                state.getPlacementManager()->addFunds(opt.initialGold);
                state.getPlacementManager()->update(0.0f);
                state.getStateManager()->switchToState(InternalState::PlaceUnits, state);
            }
        }

        ImGui::EndTable();
    }
    ImGui::End();
}
