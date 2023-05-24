#ifndef JAMTEMPLATE_WORLD_INFO_INTERFACE_HPP
#define JAMTEMPLATE_WORLD_INFO_INTERFACE_HPP

#include <vector.hpp>

class WorldInfoInterface {
public:
    virtual ~WorldInfoInterface() = default;
    virtual jt::Vector2f getClosestTargetTo(jt::Vector2f const& pos, int playerId) = 0;
};

#endif // JAMTEMPLATE_WORLD_INFO_INTERFACE_HPP
