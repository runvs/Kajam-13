#ifndef JAMTEMPLATE_PLAYER_ID_DISPATCHER_HPP
#define JAMTEMPLATE_PLAYER_ID_DISPATCHER_HPP

#include <rect.hpp>

class PlayerIdDispatcher {
public:
    explicit PlayerIdDispatcher(int playerId);

    jt::Rectf getUnitPlacementArea() const;
    jt::Rectf getBlockedUnitPlacementArea() const;

private:
    jt::Rectf m_unitPlacementRect;
    jt::Rectf m_blockedUnitPlacementArea;
};

#endif // JAMTEMPLATE_PLAYER_ID_DISPATCHER_HPP
