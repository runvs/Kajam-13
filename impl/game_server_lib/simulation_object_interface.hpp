#ifndef JAMTEMPLATE_SIMULATION_OBJECT_INTERFACE_HPP
#define JAMTEMPLATE_SIMULATION_OBJECT_INTERFACE_HPP

#include <object_properties.hpp>

class SimulationObjectInterface {
public:
    virtual ~SimulationObjectInterface() = default;
    
    virtual void update(float elapsed) = 0;
    virtual ObjectProperties saveState() const = 0;
    // TODO add a way to ask about world state
};

#endif // JAMTEMPLATE_SIMULATION_OBJECT_INTERFACE_HPP
