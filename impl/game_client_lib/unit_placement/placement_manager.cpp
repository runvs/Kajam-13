#include "placement_manager.hpp"
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <math_helper.hpp>
#include <object_group.hpp>
#include <object_properties.hpp>
#include <unit_info_collection.hpp>
#include <unit_placement/placed_unit.hpp>
#include <imgui.h>

PlacementManager::PlacementManager(int playerId,
    std::weak_ptr<PlayerIdDispatcher> playerIdDispatcher,
    std::shared_ptr<UnitInfoCollection> unitInfo)
    : m_unitInfo { unitInfo }
{
    m_playerId = playerId;
    m_playerIdDispatcher = playerIdDispatcher;
}
void PlacementManager::doCreate()
{
    auto playerIdDispatcher = m_playerIdDispatcher.lock();
    if (playerIdDispatcher == nullptr) {
        // TODO log error
        return;
    }
    getGame()->logger().info("PlacementManager with playerId: " + std::to_string(m_playerId));
    m_blockedUnitPlacementArea
        = jt::dh::createShapeRect(playerIdDispatcher->getBlockedUnitPlacementArea(),
            jt::Color { 20, 20, 20, 100 }, textureManager());
    m_placedUnits = std::make_shared<jt::ObjectGroup<PlacedUnit>>();
}
void PlacementManager::doUpdate(const float elapsed)
{
    m_placedUnitsGO.update(elapsed);
    m_placedUnits->update(elapsed);
    placeUnit();
    m_blockedUnitPlacementArea->update(elapsed);
}

void PlacementManager::doDraw() const
{
    if (!m_isActive) {
        return;
    }

    m_placedUnitsGO.draw();

    if (m_blockedUnitPlacementArea) {
        m_blockedUnitPlacementArea->draw(renderTarget());
    }

    if (m_unitInfo) {
        ImGui::Begin("unit placement");
        for (auto const& u : m_unitInfo->getUnits()) {
            if (ImGui::Button(u.type.c_str())) {
                getGame()->logger().info("select: " + u.type);
                m_activeUnitType = u.type;
            }
        }
        ImGui::End();
    }
}

void PlacementManager::placeUnit()
{
    if (m_activeUnitType == "") {
        return;
    }
    // TODO use ImGui to draw a nice UI
    if (getGame()->input().keyboard()->justPressed(jt::KeyCode::P)) {
        auto playerIdDispatcher = m_playerIdDispatcher.lock();
        if (playerIdDispatcher == nullptr) {
            // TODO log error
            return;
        }
        jt::Vector2f const mousePos = getGame()->input().mouse()->getMousePositionWorld();

        // TODO take unit size and offset into account
        if (!jt::MathHelper::checkIsIn(playerIdDispatcher->getUnitPlacementArea(), mousePos)) {
            // TODO Show some visual representation or play a sound that placing a unit here is not
            // possible.
            return;
        }

        auto unit = std::make_shared<PlacedUnit>(m_unitInfo->getInfoForType(m_activeUnitType));
        m_placedUnits->push_back(unit);
        m_placedUnitsGO.add(unit);
        unit->setGameInstance(getGame());
        unit->create();

        unit->setPosition(mousePos);

        unit->setIDs(m_unitIdManager.getIdForPlayer(m_playerId), m_playerId);
    }
}

std::vector<ObjectProperties> PlacementManager::getPlacedUnits() const
{
    std::vector<ObjectProperties> properties;
    for (auto const& u : *m_placedUnits) {
        properties.push_back(u.lock()->saveState());
    }
    return properties;
}

void PlacementManager::clearPlacedUnits() { m_placedUnitsGO.clear(); }
void PlacementManager::setActive(bool active) { m_isActive = active; }
