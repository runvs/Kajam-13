#include "placement_manager.hpp"
#include "object_properties.hpp"
#include "placed_unit.hpp"
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <math_helper.hpp>

PlacementManager::PlacementManager(
    int playerId, std::weak_ptr<PlayerIdDispatcher> playerIdDispatcher)
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
}
void PlacementManager::doUpdate(const float elapsed)
{
    for (auto& u : m_placedUnits) {
        u->update(elapsed);
    }
    placeUnit();
    m_blockedUnitPlacementArea->update(elapsed);
}

void PlacementManager::placeUnit()
{
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

        // TODO extend by unit type and other required things

        auto unit = std::make_shared<PlacedUnit>();
        m_placedUnits.push_back(unit);
        unit->setGameInstance(getGame());
        unit->create();

        unit->setPosition(mousePos);

        unit->setIDs(m_unitIdManager.getIdForPlayer(m_playerId), m_playerId);
    }
}

void PlacementManager::doDraw() const
{
    if (m_isActive) {
        for (auto const& u : m_placedUnits) {
            u->draw();
        }

        if (m_blockedUnitPlacementArea) {
            m_blockedUnitPlacementArea->draw(renderTarget());
        }
    }
}

std::vector<ObjectProperties> PlacementManager::getPlacedUnits() const
{
    std::vector<ObjectProperties> properties;
    for (auto const& u : m_placedUnits) {
        properties.push_back(u->saveState());
    }
    return properties;
}

void PlacementManager::clearPlacedUnits() { m_placedUnits.clear(); }
void PlacementManager::setActive(bool active) { m_isActive = active; }
