#ifndef JAMTEMPLATE_SIMULATION_OBJECT_INTERFACE_HPP
#define JAMTEMPLATE_SIMULATION_OBJECT_INTERFACE_HPP

#include "vector.hpp"
#include <object_properties.hpp>
#include <world_info_interface.hpp>

class SimulationObjectInterface {
public:
    virtual ~SimulationObjectInterface() = default;

    virtual void update(float elapsed, WorldInfoInterface& world) = 0;
    virtual ObjectProperties saveState() const = 0;

    virtual int getPlayerID() const = 0;
    virtual jt::Vector2f getPosition() const = 0;

    // TODO add a way to ask about world state
};

#endif // JAMTEMPLATE_SIMULATION_OBJECT_INTERFACE_HPP
