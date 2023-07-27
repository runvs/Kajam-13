#include "common_functions.hpp"
#include <game_interface.hpp>
#include <input/mouse/mouse_defines.hpp>
#include <math_helper.hpp>
#include <network_data/unit_info.hpp>
#include <rect.hpp>
#include <sprite.hpp>
#include <system_helper.hpp>
#include <unit_interface.hpp>
#include <unit_placement/placed_unit.hpp>
#include <imgui.h>
#include <string>

void CommonFunctions::updateCritters(StateGame& state)
{
    for (auto& c : *state.getCritters()) {
        auto critter = c.lock();
        if (!critter) {
            return;
        }
        if (critter->isMoving()) {
            continue;
        }
        auto const bp = critter->getPosition();
        jt::Vector2f closestPosition { -999999, -999999 };
        float closestDistance = 9999999;
        if (!state.getUnits()) {
            break;
        }

        auto const lmb = [&bp, &closestDistance, &closestPosition](auto u) {
            auto const unit = u.lock();
            if (!unit) {
                return;
            }
            auto const up = unit->getPosition();
            auto const dist = jt::MathHelper::lengthSquared(bp - up);
            if (dist < closestDistance) {
                closestDistance = dist;
                closestPosition = up;
            }
        };

        for (auto& u : *state.getUnits()) {
            lmb(u);
        }
        for (auto& u : *state.getPlacementManager()->getPlacedUnits()) {
            lmb(u);
        }

        critter->setClosestUnitPosition(closestPosition);
    }
}

namespace {
void drawUnitTooltipForOneUnit(std::shared_ptr<UnitInterface> unit, StateGame& state)
{
    if (!unit) {
        return;
    }
    auto const unitInfo = getUnitInfoWithLevelAndUpgrades(unit->getInfo(), unit->getLevel(),
        convertUpgradeInfoClientVectorToUpgradeInfoVector(
            state.getPlacementManager()->upgrades()->getBoughtUpgradesForUnit(
                unit->getPlayerID(), unit->getInfo().type)));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::BeginTooltip();
    std::string const title = unitInfo.type + " (lv. " + std::to_string(unit->getLevel()) + ")";
    ImGui::Text("%s", title.c_str());
    if (ImGui::BeginTable("ttUnitInfo", 4)) {
        ImGui::TableNextColumn();
        ImGui::Text("HP");
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        auto hpCurrent = static_cast<int>(unit->getCurrentHP());
        auto const hpMax = static_cast<int>(unitInfo.hitpointsMax);
        hpCurrent = jt::MathHelper::clamp(hpCurrent, 0, hpMax);

        if (hpCurrent == hpMax) {
            ImGui::Text("%d", hpMax);
        } else {
            ImGui::Text("%d / %d", hpCurrent, hpMax);
        }
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
        ImGui::Text("%.2f", unitInfo.damage.damage);
        ImGui::PopStyleColor();
        ImGui::TableNextColumn();
        ImGui::Text("ATS");
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 55, 0, 255));
        ImGui::Text("%.2f", unitInfo.attackTimerMax);
        ImGui::PopStyleColor();
        ImGui::TableNextColumn();
        ImGui::Text("EXP");
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
        ImGui::Text("%i", unit->getExp());
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text("/");
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
        ImGui::SameLine();
        ImGui::Text("%i", unit->getExpForLevelUp());
        ImGui::PopStyleColor();
        ImGui::EndTable();
    }
    auto& upgrades = state.getPlacementManager()->upgrades()->getBoughtUpgradesForUnit(
        unit->getPlayerID(), unit->getInfo().type);

    for (auto& upg : upgrades) {
        if (!upg.icon) {
            upg.icon = std::make_shared<jt::Sprite>(upg.info.iconPath, jt ::Recti { 0, 0, 64, 64 },
                state.getGame()->gfx().textureManager());
        }
        ImGui::Image(jt::SystemHelper::nativeHandleToImTextureId<ImTextureID>(
                         upg.icon->getSFSprite().getTexture()->getNativeHandle()),
            ImVec2 { 16.0f, 16.0f }, ImVec2 { 0.0f, 0.0f }, ImVec2 { 1.0f, 1.0f });
        ImGui::SameLine(0, -1);
        ImGui::Text("%s", upg.info.name.c_str());
    }

    ImGui::EndTooltip();
    ImGui::PopStyleVar();
}
} // namespace

void CommonFunctions::drawUnitTooltips(StateGame& state)
{
    auto const checkTooltip = [&state](auto& units) {
        for (auto& u : units) {
            auto const unit = u.lock();
            if (!unit) {
                continue;
            }
            if (unit->isMouseOver()) {
                drawUnitTooltipForOneUnit(unit, state);
                break;
            }
        }
    };
    checkTooltip(*state.getUnits());
    checkTooltip(*state.getPlacementManager()->getPlacedUnits());
}
