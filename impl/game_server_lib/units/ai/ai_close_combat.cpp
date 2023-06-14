#include "ai_close_combat.hpp"
#include <map/terrain.hpp>
#include <math_helper.hpp>
#include <random/random.hpp>
#include <units/server_unit.hpp>
#include <vector.hpp>
#include <world_info_interface.hpp>

void AiCloseCombat::update(float elapsed, ServerUnit& unit, WorldInfoInterface& world)
{
    m_attackTimer -= elapsed;
    auto t = world.getClosestTargetTo(unit.getPosition(), unit.getPlayerID());
    auto target = t.lock();

    if (!target) {
        unit.getPhysicsObject()->setVelocity(jt::Vector2f { 0.0f, 0.0f });
        return;
    }

    unit.setOffset(jt::Vector2f { 0.0f,
        -world.getTerrainMappedFieldHeight(unit.getPosition() + terrainChunkSizeInPixelHalf)
            * terrainHeightScalingFactor });

    auto dir = target->getPosition() - unit.getPosition();
    auto const dist = jt::MathHelper::length(dir);
    jt::MathHelper::normalizeMe(dir);
    auto const speedFactor = world.getLocalSpeedFactorAt(unit.getPosition(), dir);
    float speed = unit.getUnitInfoFull().movementSpeed * speedFactor;
    auto const attackRange
        = (unit.getUnitInfoFull().colliderRadius * target->getUnitInfoFull().colliderRadius) / 2.0f
        + 1.0f;
    if (dist < attackRange) {
        speed = 0;
        if (m_attackTimer <= 0) {
            m_attackTimer
                = unit.getUnitInfoFull().attackTimerMax + jt::Random::getFloat(0.0f, 0.1f);
            DamageInfo d;
            d.damage = unit.getUnitInfoFull().damage;
            target->takeDamage(d);
            if (!target->isAlive()) {
                unit.gainExperience(target->getUnitInfoFull().experienceGainWhenKilled);
            }
        }
    }

    unit.getPhysicsObject()->setVelocity(dir * speed);
}
