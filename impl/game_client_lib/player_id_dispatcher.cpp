
#include "player_id_dispatcher.hpp"
#include <game_properties.hpp>
#include <map/terrain.hpp>

PlayerIdDispatcher::PlayerIdDispatcher(int playerId)
{
    // TODO add additional flanking placement rects
    if (playerId == 0) {
        m_unitPlacementRects[AREA_MAIN] = jt::Rectf { 0, terrainFlankHeightInPixel,
            GP::GetScreenSize().x / 2, GP::GetScreenSize().y - terrainFlankHeightInPixel * 2 };
        m_unitPlacementRects[AREA_FLANK_TOP] = jt::Rectf { GP::GetScreenSize().x / 2, 0,
            GP::GetScreenSize().x, terrainFlankHeightInPixel };
        m_unitPlacementRects[AREA_FLANK_BOT] = jt::Rectf { GP::GetScreenSize().x / 2,
            GP::GetScreenSize().y - terrainFlankHeightInPixel, GP::GetScreenSize().x,
            terrainFlankHeightInPixel };
    } else {
        m_unitPlacementRects[AREA_MAIN] = jt::Rectf { GP::GetScreenSize().x / 2,
            terrainFlankHeightInPixel, GP::GetScreenSize().x / 2,
            GP::GetScreenSize().y - terrainFlankHeightInPixel * 2 };
        m_unitPlacementRects[AREA_FLANK_TOP]
            = jt::Rectf { 0, 0, GP::GetScreenSize().x / 2, terrainFlankHeightInPixel };
        m_unitPlacementRects[AREA_FLANK_BOT]
            = jt::Rectf { 0, GP::GetScreenSize().y - terrainFlankHeightInPixel,
                  GP::GetScreenSize().x / 2, terrainFlankHeightInPixel };
    }
}

jt::Rectf PlayerIdDispatcher::getUnitPlacementArea(AreaType type) const
{
    return m_unitPlacementRects[type];
}
