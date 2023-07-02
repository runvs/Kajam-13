#include "place_units.hpp"
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <graphics/drawable_interface.hpp>
#include <input/mouse/mouse_defines.hpp>
#include <internal_state/common_functions.hpp>
#include <internal_state/internal_state_manager.hpp>
#include <network_data/unit_info.hpp>
#include <rect.hpp>
#include <sprite.hpp>
#include <state_game.hpp>
#include <system_helper.hpp>
#include <unit_interface.hpp>
#include <unit_placement/placed_unit.hpp>
#include <vector2.hpp>
#include <imgui.h>
#include <string>

void PlaceUnits::update(StateGame& state, float /*elapsed*/)
{
    if (state.getGame()->input().mouse()->justPressed(jt::MouseButtonCode::MBLeft)) {
        auto const selectUnit = [this, &state](auto units) {
            for (auto& u : *units) {
                auto unit = u.lock();
                if (!unit) {
                    continue;
                }
                if (unit->getPlayerID() != state.getServerConnection()->getPlayerId()) {
                    continue;
                }
                if (unit->isMouseOver()) {
                    m_unitInterfaceSelected = unit;
                    m_selectedUnitType = unit->getInfo().type;
                    break;
                }
            }
        };
        selectUnit(state.getUnits());
        selectUnit(state.getPlacementManager()->getPlacedUnits());
    }

    auto const doHighlight = [this, pid = state.getServerConnection()->getPlayerId()](auto& units) {
        for (auto& u : units) {
            auto unit = u.lock();
            if (!unit) {
                continue;
            }
            if (unit->getPlayerID() != pid) {
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
        m_unitInterfaceSelected = nullptr;
        m_selectedUnitType.clear();
    }

    // TODO move into create function
    if (!m_imageEndPlacement) {
        m_imageEndPlacement = std::make_shared<jt::Sprite>("assets/images/menus/end_placement.png",
            jt::Recti { 0, 0, 483, 64 }, state.getGame()->gfx().textureManager());
    }
    if (!m_rangeIndicator) {
        m_rangeIndicator = jt::dh::createShapeCircle(
            64, jt::Color { 0u, 160u, 0u, 40u }, state.textureManager());
        m_rangeIndicator->setOrigin(jt::OriginMode::CENTER);
    }

    CommonFunctions::updateCritters(state);
}

void PlaceUnits::draw(StateGame& state)
{
    state.getPlacementManager()->draw();

    ImGuiWindowFlags window_flags { ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoScrollWithMouse };
    ImGui::SetNextWindowPos({ GP::GetWindowSize().x / 2 - 98, 106 }, ImGuiCond_Always);
    ImGui::SetNextWindowSize({ 196, 26 }, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
    ImGui::Begin("End Placement", nullptr, window_flags);
    ImGui::PopStyleVar(2);
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(212, 180, 134, 120));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 0));
    if (ImGui::ImageButton(jt::SystemHelper::nativeHandleToImTextureId<ImTextureID>(
                               m_imageEndPlacement->getSFSprite().getTexture()->getNativeHandle()),
            { 196, 26 }, { 0, 0 }, { 1, 1 }, 0)) {
        state.getStateManager()->switchToState(InternalState::WaitForSimulationResults, state);
    }
    ImGui::PopStyleColor(3);
    ImGui::End();

    if (m_unitInterfaceSelected) {
        drawUnitUpgradeWindow(m_unitInterfaceSelected, state);
    }

    drawUnitTooltips(state);

    drawRangeIndicator(state);
}

void PlaceUnits::drawUnitTooltips(StateGame& state)
{
    auto const checkTooltip = [this, &state](auto& units) {
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

void PlaceUnits::drawUnitTooltipForOneUnit(std::shared_ptr<UnitInterface> unit, StateGame& state)
{
    if (!unit) {
        return;
    }
    auto const unitInfo = getUnitInfoWithLevelAndUpgrades(unit->getInfo(), unit->getLevel(),
        state.getPlacementManager()->upgrades()->getBoughtUpgradesForUnit(
            unit->getPlayerID(), unit->getInfo().type));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::BeginTooltip();
    std::string const title = unitInfo.type + " (lv. " + std::to_string(unit->getLevel()) + ")";
    ImGui::Text("%s", title.c_str());
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
        //  TODO load icon on create, not during update
        if (!upg.icon) {
            upg.icon = std::make_shared<jt::Sprite>(
                upg.iconPath, jt ::Recti { 0, 0, 64, 64 }, state.getGame()->gfx().textureManager());
        }
        ImGui::Image(jt::SystemHelper::nativeHandleToImTextureId<ImTextureID>(
                         upg.icon->getSFSprite().getTexture()->getNativeHandle()),
            ImVec2 { 16.0f, 16.0f }, ImVec2 { 0.0f, 0.0f }, ImVec2 { 1.0f, 1.0f });
        ImGui::SameLine(0, -1);
        ImGui::Text("%s", upg.name.c_str());
    }

    ImGui::EndTooltip();
    ImGui::PopStyleVar();
}

namespace {

auto getWindowPos(auto const mousePosition)
{
    jt::Vector2f windowPos { mousePosition * 2.0f };
    if (mousePosition.x > GP::GetScreenSize().x / 2) {
        windowPos.x -= 240;
    }
    if (mousePosition.y > GP::GetScreenSize().y / 2) {
        windowPos.y -= 100;
    } else {
        windowPos.y += 60;
    }
    return windowPos;
}

} // namespace

void PlaceUnits::drawUnitUpgradeWindow(
    std::shared_ptr<UnitInterface> selectedUnit, StateGame& state)
{
    auto const unitType = selectedUnit->getInfo().type;
    auto& possibleUpgrades = state.getPlacementManager()->upgrades()->getPossibleUpgradesForUnit(
        selectedUnit->getPlayerID(), unitType);
    if (possibleUpgrades.empty()) {
        return;
    }
    auto const winPos = getWindowPos(state.getGame()->input().mouse()->getMousePositionScreen());
    ImGui::SetNextWindowPos({ winPos.x, winPos.y }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({ 208, 100 }, ImGuiCond_FirstUseEver);
    ImGui::Begin((unitType + " upgrades").c_str());
    for (auto& upg : possibleUpgrades) {
        if (upg.name.empty()) {
            continue;
        }
        auto const cost = upg.upgradeCost;
        auto const str = upg.name + " (" + std::to_string(cost) + ")";

        const auto canAffordUpgrade = state.getPlacementManager()->getFunds() < cost;
        ImGui::BeginDisabled(canAffordUpgrade);
        // TODO avoid loading icons in draw
        if (!upg.icon) {
            upg.icon = std::make_shared<jt::Sprite>(upg.iconPath, jt ::Recti { 0, 0, 256, 256 },
                state.getGame()->gfx().textureManager());
        }
        ImGui::Image(jt::SystemHelper::nativeHandleToImTextureId<ImTextureID>(
                         upg.icon->getSFSprite().getTexture()->getNativeHandle()),
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

            state.getPlacementManager()->upgrades()->buyUpgrade(
                selectedUnit->getPlayerID(), selectedUnit->getInfo().type, upg.name);
        }
        ImGui::EndDisabled();
    }
    ImGui::End();
}

void PlaceUnits::drawRangeIndicator(StateGame& state)
{
    auto const checkTooltip = [this, &state](auto& units) {
        for (auto& u : units) {
            auto const unit = u.lock();
            if (!unit) {
                continue;
            }
            if (unit->isMouseOver()) {
                auto const unitInfo
                    = getUnitInfoWithLevelAndUpgrades(unit->getInfo(), unit->getLevel(),
                        state.getPlacementManager()->upgrades()->getBoughtUpgradesForUnit(
                            unit->getPlayerID(), unit->getInfo().type));
                if (unitInfo.ai.type == AiInfo::AiType::CLOSE_COMBAT) {
                    continue;
                }
                auto const radius = unitInfo.ai.range * terrainChunkSizeInPixel;
                auto const scale = radius / 64.0f;
                m_rangeIndicator->setScale(jt::Vector2f { scale, scale });
                m_rangeIndicator->setPosition(
                    jt::Vector2f { unit->getPosition().x + terrainChunkSizeInPixelHalf,
                        unit->getPosition().y + terrainChunkSizeInPixelHalf });
                m_rangeIndicator->update(0.0f);
                m_rangeIndicator->draw(state.renderTarget());
                break;
            }
        }
    };
    checkTooltip(*state.getUnits());
    checkTooltip(*state.getPlacementManager()->getPlacedUnits());
}
