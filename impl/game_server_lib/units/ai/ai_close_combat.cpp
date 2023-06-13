
#include "ai_close_combat.hpp"
#include "vector.hpp"
#include <map/terrain.hpp>
#include <math_helper.hpp>
#include <units/server_unit.hpp>
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
    auto speedFactor = world.getLocalSpeedFactorAt(unit.getPosition(), dir);
    if (speedFactor == 0.0f) {
        // TODO walk around obstacle / choose another direction / path-finding?
    }
    float speed = unit.getUnitInfoFull().movementSpeed * speedFactor;
    if (dist < unit.getUnitInfoFull().colliderRadius * 2.0f) {
        speed = 0;
        if (m_attackTimer <= 0) {
            m_attackTimer = unit.getUnitInfoFull().attackTimerMax;
            DamageInfo d;
            d.damage = unit.getUnitInfoFull().damage;
            target->takeDamage(d);
            if (!target->isAlive()) {
                // TODO use real exp gain from unit
                unit.gainExperience(100);
            }
        }
    }

    unit.getPhysicsObject()->setVelocity(dir * speed);
}
