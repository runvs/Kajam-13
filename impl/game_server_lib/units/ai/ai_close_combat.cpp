
#include "ai_close_combat.hpp"
#include "vector.hpp"
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

    unit.setOffset(jt::Vector2f { 0.0f, world.getTerrainMappedFieldHeight(unit.getPosition()) });

    auto dir = target->getPosition() - unit.getPosition();
    auto const dist = jt::MathHelper::length(dir);
    jt::MathHelper::normalizeMe(dir);
    auto speedFactor = world.getLocalSpeedFactorAt(unit.getPosition(), dir);
    if (speedFactor == 0.0f) {
        // TODO walk around obstacle / choose another direction / path-finding?
    }
    float speed = unit.getInfo().movementSpeed * speedFactor;
    if (dist < unit.getInfo().colliderRadius * 2.0f) {
        speed = 0;
        if (m_attackTimer <= 0) {
            m_attackTimer = unit.getInfo().attackTimerMax;
            DamageInfo d;
            d.damage = unit.getInfo().damage;
            target->takeDamage(d);
        }
    }

    unit.getPhysicsObject()->setVelocity(dir * speed);
    //    unit.setPosition(unit.getPosition() + dir * elapsed * speed);
}
