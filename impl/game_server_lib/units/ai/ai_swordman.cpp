
#include "ai_swordman.hpp"
#include <math_helper.hpp>
#include <units/server_unit.hpp>
#include <world_info_interface.hpp>

void AiSwordman::update(float elapsed, ServerUnit& unit, WorldInfoInterface& world)
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
    float speed = unit.getInfo().movementSpeed;
    if (dist < unit.getInfo().colliderRadius * 2.0f) {
        speed = 0;
        if (m_attackTimer <= 0) {
            m_attackTimer = unit.getInfo().attackTimerMax;
            DamageInfo d;
            d.damage = unit.getInfo().damage;
            target->takeDamage(d);
        }
    }

    unit.setPosition(unit.getPosition() + dir * elapsed * speed);
}
