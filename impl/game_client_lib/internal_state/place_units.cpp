#include "place_units.hpp"
#include <game_interface.hpp>
#include <input/mouse/mouse_defines.hpp>
#include <internal_state/common_functions.hpp>
#include <internal_state/internal_state_manager.hpp>
#include <rect.hpp>
#include <sprite.hpp>
#include <state_game.hpp>
#include <unit_placement/placed_unit.hpp>
#include <imgui.h>
#include <string>

namespace {

template <typename T>
bool showUnitTooltip(T& u, PlacementManager const& pm)
{
    auto const lockedUnit = u.lock();
    if (!lockedUnit || !lockedUnit->isMouseOver()) {
        return false;
    }
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
    auto& upgrades = pm.getBoughtUpgradesForUnit((*lockedUnit).getInfo().type);
    if (!upgrades.empty()) {
        for (auto& upg : upgrades) {
            if (!upg.icon) {
                upg.icon = std::make_shared<jt::Sprite>(upg.iconPath, jt ::Recti { 0, 0, 64, 64 },
                    pm.getGame()->gfx().textureManager());
            }
            ImGui::Image(upg.icon->getSFSprite().getTexture()->getNativeHandle(),
                ImVec2 { 16.0f, 16.0f }, ImVec2 { 0.0f, 0.0f }, ImVec2 { 1.0f, 1.0f });
            ImGui::SameLine(0, -1);
            ImGui::Text("%s", upg.name.c_str());
        }
    }
    ImGui::EndTooltip();
    ImGui::PopStyleVar();
    return true;
}

template <typename T>
void drawUnitUpgrade(T& selectedUnit, StateGame& state)
{
    auto const unitType = selectedUnit->getInfo().type;
    ImGui::Begin(("Unit upgrades (" + unitType + ")").c_str());
    for (auto& upg : state.getPlacementManager()->getPossibleUpgradesForUnit(unitType)) {
        if (upg.name.empty()) {
            continue;
        }
        auto const cost = upg.upgradeCost;
        auto const str = upg.name + " (" + std::to_string(cost) + ")";

        const auto canAffordUpgrade = state.getPlacementManager()->getFunds() < cost;
        ImGui::BeginDisabled(canAffordUpgrade);
        if (!upg.icon) {
            upg.icon = std::make_shared<jt::Sprite>(upg.iconPath, jt ::Recti { 0, 0, 256, 256 },
                state.getGame()->gfx().textureManager());
        }
        ImGui::Image(upg.icon->getSFSprite().getTexture()->getNativeHandle(),
            ImVec2 { 16.0f, 16.0f }, ImVec2 { 0.0f, 0.0f }, ImVec2 { 1.0f, 1.0f });
        ImGui::SameLine(0, -1);
        if (ImGui::Button(str.c_str())) {
            state.getGame()->logger().info("clicked upgrade: " + upg.name);
            state.getPlacementManager()->addFunds(-cost);

            UpgradeUnitData data;
            data.upgrade = upg;
            data.unityType = selectedUnit->getInfo().type;
            data.playerID = selectedUnit->getPlayerID();
            state.getServerConnection()->unitUpgrade(data);
            state.flashUnitsForUpgrade(selectedUnit->getInfo().type);

            state.getPlacementManager()->buyUpgrade(selectedUnit->getInfo().type, upg.name);
        }
        ImGui::EndDisabled();
    }
    ImGui::End();
}

} // namespace

void PlaceUnits::update(StateGame& state, float elapsed)
{
    if (state.getGame()->input().mouse()->justPressed(jt::MouseButtonCode::MBLeft)) {
        for (auto& u : *state.getUnits()) {
            auto unit = u.lock();
            if (!unit) {
                continue;
            }
            if (unit->getPlayerID() != state.getServerConnection()->getPlayerId()) {
                continue;
            }
            if (unit->isMouseOver()) {
                m_selectedPlacedUnit = nullptr;
                m_selectedUnit = unit;
                m_selectedUnitType = unit->getInfo().type;
                break;
            }
        }
        for (auto& u : *state.getPlacementManager()->getPlacedUnits()) {
            auto unit = u.lock();
            if (!unit) {
                continue;
            }
            if (unit->getPlayerID() != state.getServerConnection()->getPlayerId()) {
                continue;
            }
            if (unit->isMouseOver()) {
                m_selectedUnit = nullptr;
                m_selectedPlacedUnit = unit;
                m_selectedUnitType = unit->getInfo().type;
                break;
            }
        }
    }

    auto const doHighlight = [this, &state](auto& units) {
        for (auto& u : units) {
            auto unit = u.lock();
            if (!unit) {
                continue;
            }
            if (!m_selectedUnitType.empty() && (unit->getInfo().type == m_selectedUnitType)) {
                unit->setHighlight(true);
            } else {
                unit->setHighlight(false);
            }
        }
    };
    doHighlight(*state.getUnits());
    doHighlight(*state.getPlacementManager()->getPlacedUnits());

    if (state.getGame()->input().mouse()->justPressed(jt::MouseButtonCode::MBRight)) {
        m_selectedUnit = nullptr;
        m_selectedPlacedUnit = nullptr;
        m_selectedUnitType.clear();
    }

    if (!m_rangeIndicator) {
        m_rangeIndicator = std::make_shared<sf::CircleShape>(16.0f);
        m_rangeIndicator->setFillColor({ 0, 160, 0, 40 });
        m_rangeIndicator->setOutlineColor({ 0, 160, 0, 255 });
        m_rangeIndicator->setOutlineThickness(1.0f);
        m_rangeIndicator->setPointCount(m_rangeIndicator->getPointCount() * 2);
    }

    CommonFunctions::updateCritters(state);
}

void PlaceUnits::draw(StateGame& state)
{
    state.getPlacementManager()->draw();

    if (!m_imageEndPlacement) {
        m_imageEndPlacement = std::make_shared<jt::Sprite>("assets/images/menus/end_placement.png",
            jt::Recti { 0, 0, 483, 64 }, state.getGame()->gfx().textureManager());
    }

    ImGuiWindowFlags window_flags { ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoScrollWithMouse };
    auto const wp = ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().WindowBorderSize;
    ImGui::SetNextWindowPos({ GP::GetWindowSize().x / 2 - 100 - wp / 2, 100 }, ImGuiCond_Always);
    ImGui::SetNextWindowSize({ 196 + wp, 26 }, ImGuiCond_Always);
    ImGui::Begin("End Placement", nullptr, window_flags);
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(212, 180, 134, 120));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 0));
    if (ImGui::ImageButton(m_imageEndPlacement->getSFSprite().getTexture()->getNativeHandle(),
            { 196, 26 }, { 0, 0 }, { 1, 1 }, 0)) {
        state.getStateManager()->switchToState(InternalState::WaitForSimulationResults, state);
    }
    ImGui::PopStyleColor(3);
    ImGui::End();

    if (m_selectedUnit) {
        drawUnitUpgrade(m_selectedUnit, state);
    } else if (m_selectedPlacedUnit) {
        drawUnitUpgrade(m_selectedPlacedUnit, state);
    }

    auto const checkTooltip = [this, &state](auto& units) {
        for (auto& u : units) {
            if (showUnitTooltip(u, *state.getPlacementManager())) {
                auto const unit = u.lock();
                auto const unitInfo = getUnitInfoWithLevelAndUpgrades(unit->getInfo(),
                    unit->getLevel(),
                    state.getPlacementManager()->getBoughtUpgradesForUnit(unit->getInfo().type));
                auto const radius = unitInfo.ai.range * terrainChunkSizeInPixel;
                m_rangeIndicator->setRadius(radius);
                m_rangeIndicator->setPosition(
                    unit->getPosition().x - radius + terrainChunkSizeInPixelHalf,
                    unit->getPosition().y - radius + terrainChunkSizeInPixelHalf);
                sf::RenderStates states { sf::BlendAdd };
                state.renderTarget()->get(0)->draw(*m_rangeIndicator, states);
                break;
            }
        }
    };
    checkTooltip(*state.getUnits());
    checkTooltip(*state.getPlacementManager()->getPlacedUnits());
}
