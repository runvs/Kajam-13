
#include "player_id_dispatcher.hpp"
#include <game_properties.hpp>

PlayerIdDispatcher::PlayerIdDispatcher(int playerId)
{
    if (playerId == 0) {
        m_unitPlacementRect = jt::Rectf { 0, 0, GP::GetScreenSize().x / 2, GP::GetScreenSize().y };
        m_blokedUnitPlacementArea = jt::Rectf { GP::GetScreenSize().x / 2, 0,
            GP::GetScreenSize().x / 2, GP::GetScreenSize().y };
    } else {
        m_unitPlacementRect = jt::Rectf { GP::GetScreenSize().x / 2, 0, GP::GetScreenSize().x / 2,
            GP::GetScreenSize().y };
        m_blokedUnitPlacementArea
            = jt::Rectf { 0, 0, GP::GetScreenSize().x / 2, GP::GetScreenSize().y };
    }
}

jt::Rectf PlayerIdDispatcher::getUnitPlacementArea() const { return m_unitPlacementRect; }
jt::Rectf PlayerIdDispatcher::getBlockedUnitPlacementArea() const
{
    return m_blokedUnitPlacementArea;
}
