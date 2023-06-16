#include "place_units.hpp"
#include "internal_state_manager.hpp"
#include <game_interface.hpp>
#include <input/mouse/mouse_defines.hpp>
#include <state_game.hpp>
#include <imgui.h>

void PlaceUnits::update(StateGame& state, float /*elapsed*/)
{
    if (state.getGame()->input().mouse()->justPressed(jt::MouseButtonCode::MBLeft)) {
        for (auto& u : *state.getUnits()) {
            auto unit = u.lock();
            if (unit->getPlayerID() != state.getServerConnection()->getPlayerId()) {
                continue;
            }
            if (unit->isMouseOver()) {
                m_selectedUnit = unit;
                break;
            }
        }
    }

    if (state.getGame()->input().mouse()->justPressed(jt::MouseButtonCode::MBRight)) {
        m_selectedUnit = nullptr;
    }
}
void PlaceUnits::draw(StateGame& state)
{
    state.getPlacementManager()->draw();

    ImGui::Begin("End Placement");
    if (ImGui::Button("End Placement")) {
        state.getStateManager()->switchToState(InternalState::WaitForSimulationResults, state);
    }
    ImGui::End();

    if (m_selectedUnit) {
        ImGui::Begin("Unit upgrades");
        for (auto& upg : m_selectedUnit->getInfo().possibleUpgrades) {
            if (m_selectedUnit->hasUpgrade(upg.name)) {
                continue;
            }
            auto const cost = upg.upgradeCost;
            auto const str = upg.name + " (" + std::to_string(cost) + ")";
            auto const available = state.getPlacementManager()->getFunds();
            ImGui::BeginDisabled(available < cost);
            if (ImGui::Button(str.c_str())) {
                state.getGame()->logger().info("clicked upgrade: " + upg.name);
                state.getPlacementManager()->addFunds(-cost);

                UpgradeUnitData data;
                data.upgrade = upg;
                data.unityType = m_selectedUnit->getInfo().type;
                data.playerID = m_selectedUnit->getPlayerID();
                state.getServerConnection()->unitUpgrade(data);
                // TODO add it to all units
                m_selectedUnit->addUpgrade(upg.name);
            }
            ImGui::EndDisabled();
        }
        ImGui::End();
    }
}
