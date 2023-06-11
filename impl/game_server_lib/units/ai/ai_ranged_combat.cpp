#include "ai_ranged_combat.hpp"
#include <arrow_info.hpp>
#include <damage_info.hpp>
#include <map/terrain.hpp>
#include <math_helper.hpp>
#include <units/server_unit.hpp>
#include <vector.hpp>
#include <world_info_interface.hpp>

void AiRangedCombat::update(float elapsed, ServerUnit& unit, WorldInfoInterface& world)
{
    m_attackTimer -= elapsed;
    auto t = world.getClosestTargetTo(unit.getPosition(), unit.getPlayerID());
    auto target = t.lock();
    if (!target) {
        unit.getPhysicsObject()->setVelocity(jt::Vector2f { 0.0f, 0.0f });
        return;
    }

    auto dir = target->getPosition() - unit.getPosition();
    auto const dist = jt::MathHelper::length(dir);
    jt::MathHelper::normalizeMe(dir);

    auto const attackRange = unit.getInfo().ai.range * terrainChunkSizeInPixel;
    if (dist > attackRange) {
        unit.setOffset(jt::Vector2f { 0.0f,
            -world.getTerrainMappedFieldHeight(unit.getPosition() + terrainChunkSizeInPixelHalf)
                * terrainHeightScalingFactor });
        auto speedFactor = world.getLocalSpeedFactorAt(unit.getPosition(), dir);
        if (speedFactor == 0.0f) {
            // TODO walk around obstacle / choose another direction / path-finding?
        }
        float const speed = unit.getInfo().movementSpeed * speedFactor;
        unit.getPhysicsObject()->setVelocity(dir * speed);
    } else {
        unit.getPhysicsObject()->setVelocity(jt::Vector2f { 0.0f, 0.0f });
        if (m_attackTimer <= 0) {
            m_attackTimer = unit.getInfo().attackTimerMax;
            ArrowInfo arrowInfo;
            arrowInfo.targetPlayerId = target->getPlayerID();
            arrowInfo.endPos = target->getPosition();
            arrowInfo.startPos = unit.getPosition();

            arrowInfo.damage = DamageInfo { unit.getInfo().damage };
            arrowInfo.currentPos = unit.getPosition();
            arrowInfo.totalTime = dist / unit.getInfo().ai.arrowSpeed;
            arrowInfo.maxHeight = unit.getInfo().ai.arrowHeight;

            world.spawnArrow(arrowInfo);
        }
    }
}
