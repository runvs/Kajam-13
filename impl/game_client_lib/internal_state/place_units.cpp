#include "place_units.hpp"
#include <game_interface.hpp>
#include <input/mouse/mouse_defines.hpp>
#include <internal_state/common_functions.hpp>
#include <internal_state/internal_state_manager.hpp>
#include <state_game.hpp>
#include <unit_placement/placed_unit.hpp>
#include <imgui.h>
#include <string>

namespace {

template <typename T>
void showUnitTooltip(T& u, PlacementManager const& pm)
{
    auto const lockedUnit = u.lock();
    if (lockedUnit->isMouseOver()) {
        auto const unitInfo = getUnitInfoWithLevelAndUpgrades(lockedUnit->getInfo(),
            lockedUnit->getLevel(), pm.getBoughtUpgradesForUnit(lockedUnit->getInfo().type));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
        ImGui::BeginTooltip();
        ImGui::Text("%s", unitInfo.type.c_str());
        if (ImGui::BeginTable("ttUnitInfo", 4)) {
            ImGui::TableNextColumn();
            ImGui::Text("HP");
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("%d", static_cast<int>(unitInfo.hitpointsMax));
            ImGui::PopStyleColor();
            ImGui::TableNextColumn();
            ImGui::Text("SPD");
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
            ImGui::Text("%.2f", unitInfo.movementSpeed);
            ImGui::PopStyleColor();
            ImGui::TableNextColumn();
            ImGui::Text("DMG");
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 55, 0, 255));
            ImGui::Text("%.2f", unitInfo.damage);
            ImGui::PopStyleColor();
            ImGui::TableNextColumn();
            ImGui::Text("ATS");
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 55, 0, 255));
            ImGui::Text("%.2f", unitInfo.attackTimerMax);
            ImGui::PopStyleColor();
            ImGui::EndTable();
        }
        auto const upgrades = pm.getBoughtUpgradeNamesForUnit((*lockedUnit).getInfo().type);
        if (!upgrades.empty()) {
            ImGui::Text("Upgrades:");
            for (auto const& upg : upgrades) {
                ImGui::BulletText("%s", upg.c_str());
            }
        }
        ImGui::EndTooltip();
        ImGui::PopStyleVar();
    }
}

template <typename T>
void drawUnitUpgrade(T& selectedUnit, StateGame& state)
{
    ImGui::Begin("Unit upgrades");
    auto const unitType = selectedUnit->getInfo().type;
    for (auto& upgName : state.getPlacementManager()->getPossibleUpgradesForUnit(unitType)) {
        auto const upg = state.getUnitInfo()->getUpgradeForUnit(unitType, upgName);
        auto const cost = upg.upgradeCost;
        auto const str = upgName + " (" + std::to_string(cost) + ")";

        const auto canAffordUpgrade = state.getPlacementManager()->getFunds() < cost;
        ImGui::BeginDisabled(canAffordUpgrade);
        if (ImGui::Button(str.c_str())) {
            state.getGame()->logger().info("clicked upgrade: " + upgName);
            state.getPlacementManager()->addFunds(-cost);
            state.getPlacementManager()->buyUpgrade(selectedUnit->getInfo().type, upgName);

            UpgradeUnitData data;
            data.upgrade = upg;
            data.unityType = selectedUnit->getInfo().type;
            data.playerID = selectedUnit->getPlayerID();
            state.getServerConnection()->unitUpgrade(data);
            for (auto& u : *state.getUnits()) {
                u.lock()->addUpgrade(upg.name);
            }
        }
        ImGui::EndDisabled();
    }
    ImGui::End();
}

} // namespace

void PlaceUnits::update(StateGame& state, float /*elapsed*/)
{
    if (state.getGame()->input().mouse()->justPressed(jt::MouseButtonCode::MBLeft)) {
        for (auto& u : *state.getUnits()) {
            auto unit = u.lock();
            if (unit->getPlayerID() != state.getServerConnection()->getPlayerId()) {
                continue;
            }
            if (unit->isMouseOver()) {
                m_selectedPlacedUnit = nullptr;
                m_selectedUnit = unit;
                break;
            }
        }
        for (auto& u : *state.getPlacementManager()->getPlacedUnits()) {
            auto unit = u.lock();
            if (unit->getPlayerID() != state.getServerConnection()->getPlayerId()) {
                continue;
            }
            if (unit->isMouseOver()) {
                m_selectedUnit = nullptr;
                m_selectedPlacedUnit = unit;
                break;
            }
        }
    }

    if (state.getGame()->input().mouse()->justPressed(jt::MouseButtonCode::MBRight)) {
        m_selectedUnit = nullptr;
        m_selectedPlacedUnit = nullptr;
    }

    CommonFunctions::updateBirds(state);
}

void PlaceUnits::draw(StateGame& state)
{
    for (auto& u : *state.getUnits()) {
        showUnitTooltip(u, *state.getPlacementManager());
    }
    for (auto& u : *state.getPlacementManager()->getPlacedUnits()) {
        showUnitTooltip(u, *state.getPlacementManager());
    }

    state.getPlacementManager()->draw();

    ImGui::Begin("End Placement");
    if (ImGui::Button("End Placement")) {
        state.getStateManager()->switchToState(InternalState::WaitForSimulationResults, state);
    }
    ImGui::End();

    if (m_selectedUnit) {
        drawUnitUpgrade(m_selectedUnit, state);
    } else if (m_selectedPlacedUnit) {
        drawUnitUpgrade(m_selectedPlacedUnit, state);
    }
}
