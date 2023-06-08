#include "ai_archer.hpp"
#include "arrow_info.hpp"
#include "damage_info.hpp"
#include "map/terrain.hpp"
#include <math_helper.hpp>
#include <units/server_unit.hpp>
#include <world_info_interface.hpp>

void AiArcher::update(float elapsed, ServerUnit& unit, WorldInfoInterface& world)
{
    m_attackTimer -= elapsed;
    auto t = world.getClosestTargetTo(unit.getPosition(), unit.getPlayerID());
    auto target = t.lock();
    if (!target) {
        return;
    }

    auto dir = target->getPosition() - unit.getPosition();
    auto const dist = jt::MathHelper::length(dir);
    jt::MathHelper::normalizeMe(dir);

    // TODO move attackrange to unitInfo and json
    auto const attackRange = terrainChunkSizeInPixel * 10;
    if (dist > attackRange) {
        float speed = 20.0f * unit.getInfo().movementSpeed;

        unit.setPosition(unit.getPosition() + dir * elapsed * speed);
    } else {
        if (m_attackTimer <= 0) {
            m_attackTimer = unit.getInfo().attackTimerMax;
            ArrowInfo arrowInfo;
            arrowInfo.targetPlayerId = target->getPlayerID();
            arrowInfo.endPos = target->getPosition();
            arrowInfo.startPos = unit.getPosition();

            arrowInfo.damage = DamageInfo { unit.getInfo().damage };
            arrowInfo.currentPos = unit.getPosition();
            // TODO move to GP
            float const arrowSpeed = 80.0f;
            arrowInfo.totalTime = dist / arrowSpeed;
            world.spawnArrow(arrowInfo);
        }
    }
}
