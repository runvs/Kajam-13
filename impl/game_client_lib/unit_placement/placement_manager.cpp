#include "placement_manager.hpp"
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <map/terrain.hpp>
#include <math_helper.hpp>
#include <object_group.hpp>
#include <object_properties.hpp>
#include <unit_info_collection.hpp>
#include <unit_placement/placed_unit.hpp>
#include <vector.hpp>
#include <imgui.h>
#include <iostream>

PlacementManager::PlacementManager(std::shared_ptr<Terrain> world, int playerId,
    std::weak_ptr<PlayerIdDispatcher> playerIdDispatcher,
    std::shared_ptr<UnitInfoCollection> unitInfo)
    : m_world { world }
    , m_unitInfo { unitInfo }
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
            getGame()->logger().warning(
                "place unit with invalid playerIDDispatcher called", { "PlacementManager" });
            return;
        }

        jt::Vector2f fieldPos { m_world->getMappedFieldPosition(
            getGame()->input().mouse()->getMousePositionWorld()) };
        fieldPos = jt::Vector2f { static_cast<float>(terrainChunkSizeInPixel
                                      * static_cast<int>(fieldPos.x / terrainChunkSizeInPixel)),
            static_cast<float>(
                terrainChunkSizeInPixel * static_cast<int>(fieldPos.y / terrainChunkSizeInPixel)) };
        // TODO take unit size into account
        if (!jt::MathHelper::checkIsIn(playerIdDispatcher->getUnitPlacementArea(), fieldPos)
            // only one unit per field
            || fieldInUse(fieldPos)) {
            // TODO Show some visual representation or play a sound that placing a unit here is not
            getGame()->logger().info(
                "tried to place unit in invalid position", { "PlacementManager" });
            // possible.
            return;
        }

        auto unit = std::make_shared<PlacedUnit>(m_unitInfo->getInfoForType(m_activeUnitType));
        m_placedUnits->push_back(unit);
        m_placedUnitsGO.add(unit);
        unit->setGameInstance(getGame());
        unit->create();

        unit->setOffset(jt::Vector2f {
            0, -1.0f * m_world->getFieldHeight(fieldPos) * terrainHeightScalingFactor });
        unit->setPosition(fieldPos);

        unit->setIDs(m_unitIdManager.getIdForPlayer(m_playerId), m_playerId);
    }
}

bool PlacementManager::fieldInUse(jt::Vector2f const& pos) const
{
    auto const posx = static_cast<int>(pos.x / terrainChunkSizeInPixel);
    auto const posy = static_cast<int>(pos.y / terrainChunkSizeInPixel);
    for (auto const& unit : *m_placedUnits) {
        auto const lockedUnit = unit.lock();
        auto const unitx = static_cast<int>(lockedUnit->getPosition().x / terrainChunkSizeInPixel);
        auto const unity = static_cast<int>(lockedUnit->getPosition().y / terrainChunkSizeInPixel);
        if (posx == unitx && posy == unity) {
            return true;
        }
    }
    return false;
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
