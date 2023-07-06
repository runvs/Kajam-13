#include "ai_ranged_combat.hpp"
#include <map/terrain.hpp>
#include <math_helper.hpp>
#include <network_data/arrow_info.hpp>
#include <random/random.hpp>
#include <units/server_unit.hpp>
#include <world_info_interface.hpp>

AiRangedCombat::AiRangedCombat() { m_attackTimer = jt::Random::getFloat(0.3f, 0.6f); }

void AiRangedCombat::update(float elapsed, ServerUnit* unit, WorldInfoInterface& world)
{
    m_attackTimer -= elapsed;
    auto t = world.getClosestTargetTo(unit->getPosition(), unit->getPlayerID());
    auto target = t.lock();
    if (!target) {
        unit->getPhysicsObject()->setVelocity(jt::Vector2f { 0.0f, 0.0f });
        return;
    }

    auto dir = target->getPosition() - unit->getPosition();
    auto const dist = jt::MathHelper::length(dir);
    jt::MathHelper::normalizeMe(dir);

    auto const attackRange = unit->getUnitInfoFull().ai.range * terrainChunkSizeInPixel
        + world.getTerrainMappedFieldHeight(unit->getPosition()) * 10.0f;
    if (dist > attackRange) {
        unit->setOffset(jt::Vector2f { 0.0f,
            -world.getTerrainMappedFieldHeight(unit->getPosition() + terrainChunkSizeInPixelHalf)
                * terrainHeightScalingFactor });
        auto const speedFactor = world.getLocalSpeedFactorAt(unit->getPosition(), dir);
        float const speed = unit->getUnitInfoFull().movementSpeed * speedFactor;
        unit->getPhysicsObject()->setVelocity(dir * speed);
        unit->setAnim("walk");
    } else {
        unit->getPhysicsObject()->setVelocity(jt::Vector2f { 0.0f, 0.0f });
        if (m_attackTimer <= 0) {
            unit->setAnim("attack");
            m_attackTimer
                = unit->getUnitInfoFull().attackTimerMax + jt::Random::getFloat(0.0f, 0.2f);
            ArrowInfo arrowInfo;
            arrowInfo.targetPlayerId = target->getPlayerID();
            arrowInfo.endPos = target->getPosition();
            arrowInfo.startPos = unit->getPosition() + unit->getOffset();
            if (arrowInfo.endPos.x > arrowInfo.startPos.x) {
                arrowInfo.startPos += jt::Vector2f { 9.0f, 4.0f };
            } else {
                arrowInfo.startPos += jt::Vector2f { 2.0f, 3.0f };
            }

            arrowInfo.damage = unit->getUnitInfoFull().damage;
            arrowInfo.currentPos = unit->getPosition();
            arrowInfo.totalTime = dist / unit->getUnitInfoFull().ai.arrowSpeed;
            auto const heightScaling = jt::MathHelper::clamp(dist / attackRange, 0.05f, 1.0f);
            arrowInfo.maxHeight = heightScaling * unit->getUnitInfoFull().ai.arrowHeight;

            arrowInfo.shooterPlayerId = unit->getPlayerID();
            arrowInfo.shooterUnitId = unit->getUnitID();

            arrowInfo.splashRadius = 0.0f;
            arrowInfo.arrowScale = 1.0f;

            world.spawnArrow(arrowInfo, 0.81f);
        }
    }
}
