#ifndef JAMTEMPLATE_WORLD_INFO_INTERFACE_HPP
#define JAMTEMPLATE_WORLD_INFO_INTERFACE_HPP

#include <close_combat_info.hpp>
#include <network_data/arrow_info.hpp>
#include <simulation_object_interface.hpp>
#include <vector2.hpp>
#include <memory>

class SimulationObjectInterface;

class WorldInfoInterface {
public:
    virtual ~WorldInfoInterface() = default;
    virtual std::weak_ptr<SimulationObjectInterface> getClosestTargetTo(
        jt::Vector2f const& pos, int playerId)
        = 0;
    virtual float getLocalSlope(jt::Vector2f const& pos, jt::Vector2f const& direction) = 0;
    virtual jt::Vector2f getTerrainMappedFieldPosition(jt::Vector2f const& pos, int& x, int& y) = 0;
    virtual float getTerrainMappedFieldHeight(jt::Vector2f const& pos) = 0;
    virtual void spawnArrow(ArrowInfo const& arrowInfo, float delay) = 0;
    virtual void scheduleAttack(CloseCombatInfo const& info, float delay) = 0;
};

#endif // JAMTEMPLATE_WORLD_INFO_INTERFACE_HPP
