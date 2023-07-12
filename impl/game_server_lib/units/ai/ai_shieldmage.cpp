#include "ai_shieldmage.hpp"
#include "map/terrain.hpp"
#include <network_data/shield_info.hpp>
#include <vector2.hpp>
#include <world_info_interface.hpp>

void AiShieldmage::update(float elapsed, ServerUnit* unit, WorldInfoInterface& world)
{
    if (!m_shieldTriggerd) {
        m_initialPos = unit->getPosition();
        m_shieldTriggerd = true;
        ShieldInfo si;
        si.pos = unit->getPosition() + unit->getOffset()
            + jt::Vector2f { terrainChunkSizeInPixelHalf, terrainChunkSizeInPixelHalf };
        si.unitID = unit->getUnitID();
        si.playerID = unit->getPlayerID();
        si.radius = unit->getUnitInfoFull().ai.range * terrainChunkSizeInPixel;
        si.hpCurrent = unit->getUnitInfoFull().ai.shieldHp;
        si.hpMax = unit->getUnitInfoFull().ai.shieldHp;
        world.spawnShield(si);
    }
    unit->setAnim("spell");
    unit->setPosition(m_initialPos);
    unit->getPhysicsObject()->setVelocity(jt::Vector2f { 0.0f, 0.0f });
}
