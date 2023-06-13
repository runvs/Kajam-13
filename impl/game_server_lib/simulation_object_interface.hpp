#ifndef JAMTEMPLATE_SIMULATION_OBJECT_INTERFACE_HPP
#define JAMTEMPLATE_SIMULATION_OBJECT_INTERFACE_HPP

#include "damage_info.hpp"
#include <object_properties.hpp>
#include <vector.hpp>

class WorldInfoInterface;

class SimulationObjectInterface {
public:
    virtual ~SimulationObjectInterface() = default;

    virtual void update(float elapsed, WorldInfoInterface& world) = 0;

    virtual ObjectProperties saveState() const = 0;
    virtual void updateState(ObjectProperties* props) = 0;
    virtual void levelUnitUp() = 0;
    virtual int getLevel() const = 0;

    virtual int getPlayerID() const = 0;
    virtual int getUnitID() const = 0;
    virtual jt::Vector2f getPosition() const = 0;
    virtual jt::Vector2f getOffset() const = 0;

    virtual bool isAlive() const = 0;

    virtual void takeDamage(DamageInfo const& damage) = 0;
    virtual int getCost() = 0;
    virtual void gainExperience(int exp) = 0;
};

#endif // JAMTEMPLATE_SIMULATION_OBJECT_INTERFACE_HPP
