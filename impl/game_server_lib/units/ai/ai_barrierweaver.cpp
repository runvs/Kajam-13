#include "ai_barrierweaver.hpp"
#include "map/terrain.hpp"
#include <network_data/barrier_info.hpp>
#include <vector2.hpp>
#include <world_info_interface.hpp>

void AiBarrierWeaver::update(float elapsed, ServerUnit* unit, WorldInfoInterface& world)
{
    if (!m_barrierTriggerd) {
        m_initialPos = unit->getPosition();
        m_barrierTriggerd = true;
        BarrierInfo bi;
        bi.pos = unit->getPosition() + unit->getOffset()
            + jt::Vector2f { terrainChunkSizeInPixelHalf, terrainChunkSizeInPixelHalf };
        bi.unitID = unit->getUnitID();
        bi.playerID = unit->getPlayerID();
        bi.radius = unit->getUnitInfoFull().ai.range * terrainChunkSizeInPixel;
        bi.hpCurrent = unit->getUnitInfoFull().ai.barrierHp;
        bi.hpMax = unit->getUnitInfoFull().ai.barrierHp;
        world.spawnBarrier(bi);
    }
    unit->setAnim("spell");
    unit->setPosition(m_initialPos);
    unit->getPhysicsObject()->setVelocity(jt::Vector2f { 0.0f, 0.0f });
}
