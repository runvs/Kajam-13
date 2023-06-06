#ifndef JAMTEMPLATE_WORLD_INFO_INTERFACE_HPP
#define JAMTEMPLATE_WORLD_INFO_INTERFACE_HPP

#include <simulation_object_interface.hpp>
#include <vector.hpp>
#include <memory>

class SimulationObjectInterface;

class WorldInfoInterface {
public:
    virtual ~WorldInfoInterface() = default;
    virtual std::weak_ptr<SimulationObjectInterface> getClosestTargetTo(
        jt::Vector2f const& pos, int playerId)
        = 0;
};

#endif // JAMTEMPLATE_WORLD_INFO_INTERFACE_HPP
