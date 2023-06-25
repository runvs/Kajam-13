#include "placement_area.hpp"
#include <game_properties.hpp>
#include <map/terrain.hpp>

namespace {

// clang-format off
jt::Rectf unitPlacementRects[2][3] {
    // player 0
    {
        // main
        { 0, terrainFlankHeightInPixel, GP::GetScreenSize().x / 2,
            GP::GetScreenSize().y - terrainFlankHeightInPixel * 2 },
        // flanking top
        { GP::GetScreenSize().x / 2, 0, GP::GetScreenSize().x, terrainFlankHeightInPixel },
        // flanking bot
        { GP::GetScreenSize().x / 2, GP::GetScreenSize().y - terrainFlankHeightInPixel,
            GP::GetScreenSize().x, terrainFlankHeightInPixel }
    },
    // player 1
    {
        // main
        { GP::GetScreenSize().x / 2, terrainFlankHeightInPixel, GP::GetScreenSize().x / 2,
            GP::GetScreenSize().y - terrainFlankHeightInPixel * 2 },
        // flanking top
        { 0, 0, GP::GetScreenSize().x / 2, terrainFlankHeightInPixel },
        // flanking bot
        { 0, GP::GetScreenSize().y - terrainFlankHeightInPixel, GP::GetScreenSize().x / 2,
            terrainFlankHeightInPixel }
    }
};
// clang-format on

} // namespace

jt::Rectf getUnitPlacementArea(int const playerId, AreaType const type)
{
    return unitPlacementRects[playerId][type];
}
