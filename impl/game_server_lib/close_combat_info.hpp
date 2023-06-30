#ifndef JAMTEMPLATE_CLOSE_COMBAT_INFO_HPP
#define JAMTEMPLATE_CLOSE_COMBAT_INFO_HPP

#include <network_data/damage_info.hpp>
#include <simulation_object_interface.hpp>
#include <units/server_unit.hpp>

struct CloseCombatInfo {
    ServerUnit* attacker;
    SimulationObjectInterface* attacked;
    DamageInfo damage;
};

#endif // JAMTEMPLATE_CLOSE_COMBAT_INFO_HPP
