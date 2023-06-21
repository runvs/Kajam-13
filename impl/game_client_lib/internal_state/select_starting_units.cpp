#include "select_starting_units.hpp"
#include "state_game.hpp"
#include <imgui.h>

namespace {
// TODO move to separate file
struct SelectUnitInfo {
    std::string unit1;
    std::string unit2;
    int initialGold;
    std::string effect { "No Benefit" };
};
} // namespace

void SelectStartingUnits::update(StateGame& /*state*/, float /*elapsed*/) { }
void SelectStartingUnits::draw(StateGame& state)
{
    ImGui::Begin("Select Starting Units");

    std::vector<SelectUnitInfo> options;
    // TODO move to json files for parsing and easy configurability
    {
        SelectUnitInfo option;
        option.unit1 = "peasant";
        option.unit2 = "cannon";
        option.initialGold = 135;
        options.push_back(option);
    }

    {
        SelectUnitInfo option;
        option.unit1 = "archer";
        option.unit2 = "swordman";
        option.initialGold = 175;
        options.push_back(option);
    }

    {
        SelectUnitInfo option;
        option.unit1 = "shieldman";
        option.unit2 = "swordman";
        option.initialGold = 225;
        options.push_back(option);
    }

    if (ImGui::BeginTable("Startout Options", 5)) {

        for (auto i = 0u; i != options.size(); ++i) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", options[i].unit1.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", options[i].unit2.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%i", options[i].initialGold);
            ImGui::TableNextColumn();
            ImGui::Text("%s", options[i].effect.c_str());
            ImGui::TableNextColumn();
            auto buttonText = "Buy##" + std::to_string(i);
            if (ImGui::Button(buttonText.c_str())) {
                state.getPlacementManager()->unlockType(options[i].unit1);
                state.getPlacementManager()->unlockType(options[i].unit2);
                state.getPlacementManager()->addFunds(options[i].initialGold);
                state.getPlacementManager()->update(0.0f);
                state.getStateManager()->switchToState(InternalState::PlaceUnits, state);
            }
        }

        ImGui::EndTable();
    }
    ImGui::End();
}
