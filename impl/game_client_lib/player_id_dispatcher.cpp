
#include "player_id_dispatcher.hpp"
#include <game_properties.hpp>
#include <map/terrain.hpp>

PlayerIdDispatcher::PlayerIdDispatcher(int playerId)
{
    // TODO add additional flanking placement rects
    if (playerId == 0) {
        m_unitPlacementRect = jt::Rectf { 0, terrainFlankHeightInPixel, GP::GetScreenSize().x / 2,
            GP::GetScreenSize().y - terrainFlankHeightInPixel };
        m_blockedUnitPlacementArea = jt::Rectf { GP::GetScreenSize().x / 2, 0,
            GP::GetScreenSize().x / 2, GP::GetScreenSize().y * 2 };
    } else {
        m_unitPlacementRect = jt::Rectf { GP::GetScreenSize().x / 2, terrainFlankHeightInPixel,
            GP::GetScreenSize().x / 2, GP::GetScreenSize().y - terrainFlankHeightInPixel * 2 };
        m_blockedUnitPlacementArea
            = jt::Rectf { 0, 0, GP::GetScreenSize().x / 2, GP::GetScreenSize().y };
    }
}

jt::Rectf PlayerIdDispatcher::getUnitPlacementArea() const { return m_unitPlacementRect; }
jt::Rectf PlayerIdDispatcher::getBlockedUnitPlacementArea() const
{
    return m_blockedUnitPlacementArea;
}
