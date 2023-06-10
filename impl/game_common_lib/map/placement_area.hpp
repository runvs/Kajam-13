#ifndef JAMTEMPLATE_PLACEMENT_AREA_HPP
#define JAMTEMPLATE_PLACEMENT_AREA_HPP

#include <rect.hpp>

enum AreaType { AREA_MAIN, AREA_FLANK_TOP, AREA_FLANK_BOT };

jt::Rectf getUnitPlacementArea(int const playerId, AreaType const type);

#endif // JAMTEMPLATE_PLACEMENT_AREA_HPP
