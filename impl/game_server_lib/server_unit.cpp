#include "server_unit.hpp"
#include <damage_info.hpp>
#include <json_keys.hpp>
#include <math_helper.hpp>
#include <object_properties.hpp>
#include <world_info_interface.hpp>
#include <cmath>

ServerUnit::ServerUnit(const UnitInfo& info) { m_info = info; }

ObjectProperties ServerUnit::saveState() const
{
    ObjectProperties props;
    props.ints[jk::unitID] = m_unitID;
    props.ints[jk::playerID] = m_playerID;
    props.floats[jk::positionX] = m_pos.x;
    props.floats[jk::positionY] = m_pos.y;
    props.floats[jk::hpCurrent] = m_hp;
    props.strings[jk::unitType] = m_info.type;
    if (!m_newAnim.empty()) {
        props.strings[jk::unitAnim] = m_newAnim;
        m_newAnim == "";
    }
    return props;
}

void ServerUnit::updateState(ObjectProperties const& props)
{
    m_unitID = props.ints.at(jk::unitID);
    m_playerID = props.ints.at(jk::playerID);
    m_pos = jt::Vector2f { props.floats.at(jk::positionX), props.floats.at(jk::positionY) };
}

void ServerUnit::update(float elapsed, WorldInfoInterface& world)
{
    m_age += elapsed;
    if (!isAlive()) {
        return;
    }

    // TODO move to AI
    // determine target
    m_attackTimer -= elapsed;
    auto t = world.getClosestTargetTo(m_pos, m_playerID);
    auto target = t.lock();
    if (!target) {
        return;
    }
    auto dir = target->getPosition() - getPosition();
    auto const dist = jt::MathHelper::length(dir);
    jt::MathHelper::normalizeMe(dir);
    float speed = 20.0f * m_info.movementSpeed;
    if (dist < m_info.colliderRadius * 2.0f) {
        speed = 0;
        if (m_attackTimer <= 0) {
            // TODO take damage and attackTimer value from json
            m_attackTimer = 1.0f;
            DamageInfo d;
            d.damage = 20.0f;
            target->takeDamage(d);
        }
    }

    m_pos += dir * elapsed * speed;
}

void ServerUnit::setPosition(jt::Vector2f const& pos) { m_pos = pos; }

jt::Vector2f ServerUnit::getPosition() const { return m_pos; }
void ServerUnit::setUnitID(int unitID) { m_unitID = unitID; }
int ServerUnit::getPlayerID() const { return m_playerID; }
int ServerUnit::getUnitID() const { return m_unitID; }
void ServerUnit::takeDamage(const DamageInfo& damage)
{
    // TODO take armor into account
    m_hp -= damage.damage;
    if (m_hp > 0) {
        m_newAnim = "damage";
    } else {
        m_newAnim = "death";
    }
}
bool ServerUnit::isAlive() const { return m_hp > 0; }
