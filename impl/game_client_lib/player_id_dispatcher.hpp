#ifndef JAMTEMPLATE_PLAYER_ID_DISPATCHER_HPP
#define JAMTEMPLATE_PLAYER_ID_DISPATCHER_HPP

#include <rect.hpp>

class PlayerIdDispatcher {
public:
    enum AreaType { AREA_MAIN, AREA_FLANK_TOP, AREA_FLANK_BOT };

    explicit PlayerIdDispatcher(int playerId);

    jt::Rectf getUnitPlacementArea(AreaType type) const;

private:
    jt::Rectf m_unitPlacementRects[3];
};

#endif // JAMTEMPLATE_PLAYER_ID_DISPATCHER_HPP
