#include "place_units.hpp"
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <graphics/drawable_interface.hpp>
#include <input/mouse/mouse_defines.hpp>
#include <internal_state/common_functions.hpp>
#include <internal_state/internal_state_manager.hpp>
#include <map/terrain.hpp>
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
    if (m_deleteUnitOnclick) {
        if (state.getPlacementManager()->getActiveUnitType() != "") {
            m_deleteUnitOnclick = false;
        }
    }
    if (state.getGame()->input().mouse()->justPressed(jt::MouseButtonCode::MBLeft)) {
        if (m_deleteUnitOnclick) {
            auto const& mouse = state.getGame()->input().mouse();
            if (mouse->justPressed(jt::MouseButtonCode::MBLeft)) {
                auto mousePos = mouse->getMousePositionWorld();
                mousePos = { std::round(mousePos.x), std::round(mousePos.y) };
                int mousePosX { 0 };
                int mousePosY { 0 };
                state.getTerrain()->getMappedFieldPosition(mousePos, mousePosX, mousePosY);
                state.getGame()->logger().warning("sell Click for mouse: "
                    + std::to_string(mousePosX) + "," + std::to_string(mousePosY) + " "
                    + std::to_string(mousePos.x) + "," + std::to_string(mousePos.y));
                for (auto const& u : *state.getUnits()) {
                    auto const unit = u.lock();
                    if (!unit) {
                        continue;
                    }

                    if (unit->getPlayerID() != state.getServerConnection()->getPlayerId()) {
                        continue;
                    }

                    int unitPosX { 0 };
                    int unitPosY { 0 };
                    state.getTerrain()->getMappedFieldPosition(
                        unit->getPosition(), unitPosX, unitPosY);
                    if ((unitPosX == mousePosX) && (unitPosY == mousePosY)) {
                        state.removeUnit(unit->getUnitID());
                        state.getPlacementManager()->fieldInUse(unitPosX, unitPosY) = false;
                        state.getGame()->logger().warning("removing unit: "
                            + std::to_string(unitPosX) + "," + std::to_string(unitPosY)
                            + " ID: " + std::to_string(unit->getUnitID()));
                        state.getPlacementManager()->sellUnitForPreviousRound(unit->getInfo());
                        break;
                    }
                }
            }
        } else {
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
        m_deleteUnitOnclick = false;
    }

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

    ImGui::Begin("Unit placement");
    ImGui::Separator();
    bool const canSellUnits = !state.getUnits()->empty();
    ImGui::BeginDisabled(!canSellUnits);
    if (!m_deleteUnitOnclick) {
        if (ImGui::Button("Sell unit")) {
            m_deleteUnitOnclick = true;
            m_unitInterfaceSelected = nullptr;
            m_selectedUnitType.clear();
            state.getPlacementManager()->resetActiveUnitType();
        }
    } else {
        if (ImGui::Button("Cancel unit selling")) {
            m_deleteUnitOnclick = false;
        }
    }
    ImGui::EndDisabled();
    ImGui::End();

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

    CommonFunctions::drawUnitTooltips(state);

    drawRangeIndicator(state);
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

void PlaceUnits::create(StateGame& /*state*/) { }

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
        if (upg.info.name.empty()) {
            continue;
        }
        auto const cost = upg.info.upgradeCost;
        auto const str = upg.info.name + " (" + std::to_string(cost) + ")";

        auto const canAffordUpgrade = state.getPlacementManager()->getFunds() < cost;
        ImGui::BeginDisabled(canAffordUpgrade);
        if (!upg.icon) {
            upg.icon = std::make_shared<jt::Sprite>(upg.info.iconPath,
                jt ::Recti { 0, 0, 256, 256 }, state.getGame()->gfx().textureManager());
        }
        ImGui::Image(jt::SystemHelper::nativeHandleToImTextureId<ImTextureID>(
                         upg.icon->getSFSprite().getTexture()->getNativeHandle()),
            ImVec2 { 16.0f, 16.0f }, ImVec2 { 0.0f, 0.0f }, ImVec2 { 1.0f, 1.0f });
        ImGui::SameLine(0, -1);
        if (ImGui::Button(str.c_str())) {
            state.getGame()->logger().info("clicked upgrade: " + upg.info.name);
            state.getPlacementManager()->spendFunds(cost);

            UpgradeUnitData data;
            data.upgrade = upg.info;
            data.unityType = selectedUnit->getInfo().type;
            data.playerID = selectedUnit->getPlayerID();
            state.getServerConnection()->unitUpgrade(data);
            state.flashUnitsForUpgrade(selectedUnit->getInfo().type);

            state.getPlacementManager()->upgrades()->buyUpgrade(
                selectedUnit->getPlayerID(), selectedUnit->getInfo().type, upg.info.name);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            ImGui::SetTooltip("%s", upg.info.description.c_str());
        }
        ImGui::EndDisabled();
    }
    ImGui::End();
}

void PlaceUnits::drawRangeIndicator(StateGame& state)
{
    auto const drawRangeIndicator = [this, &state](auto const& unitInfo, auto const& pos) {
        auto const radius = unitInfo.ai.range * terrainChunkSizeInPixel;
        auto const scale = radius / 64.0f;
        m_rangeIndicator->setScale(jt::Vector2f { scale, scale });
        m_rangeIndicator->setPosition(pos);
        m_rangeIndicator->update(0.0f);
        m_rangeIndicator->draw(state.renderTarget());
    };

    // first check if we have an active type that can be placed and draw a range indicator for it
    auto const activeUnitType = state.getPlacementManager()->getActiveUnitType();
    if (!activeUnitType.empty()) {
        auto const unitInfo
            = state.getPlacementManager()->getUnitInfoCollection()->getInfoForType(activeUnitType);
        if (unitInfo.ai.type != AiInfo::AiType::CLOSE_COMBAT) {
            int posX, posY;
            auto fieldPos = state.getTerrain()->getMappedFieldPosition(
                state.getGame()->input().mouse()->getMousePositionWorld(), posX, posY);
            if (state.getPlacementManager()->canUnitBePlacedInField(fieldPos, posX, posY)) {
                drawRangeIndicator(unitInfo,
                    jt::Vector2f { static_cast<float>(posX) * terrainChunkSizeInPixel
                            + terrainChunkSizeInPixelHalf,
                        static_cast<float>(posY) * terrainChunkSizeInPixel
                            + terrainChunkSizeInPixelHalf });
                // do not show tooltips for already placed units when we have an ongoing placement
                return;
            }
        }
    }

    auto const checkTooltip = [&state, &drawRangeIndicator](auto& units) {
        for (auto& u : units) {
            auto const unit = u.lock();
            if (!unit) {
                continue;
            }
            if (unit->isMouseOver()) {
                auto const unitInfo
                    = getUnitInfoWithLevelAndUpgrades(unit->getInfo(), unit->getLevel(),
                        convertUpgradeInfoClientVectorToUpgradeInfoVector(
                            state.getPlacementManager()->upgrades()->getBoughtUpgradesForUnit(
                                unit->getPlayerID(), unit->getInfo().type)));
                if (unitInfo.ai.type == AiInfo::AiType::CLOSE_COMBAT) {
                    continue;
                }
                drawRangeIndicator(unitInfo,
                    jt::Vector2f { unit->getPosition().x + terrainChunkSizeInPixelHalf,
                        unit->getPosition().y + terrainChunkSizeInPixelHalf });
                break;
            }
        }
    };
    checkTooltip(*state.getUnits());
    checkTooltip(*state.getPlacementManager()->getPlacedUnits());
}
