#include "server_unit.hpp"
#include "ai/ai_archer.hpp"
#include "ai/ai_swordman.hpp"
#include "damage_info.hpp"
#include "json_keys.hpp"
#include "math_helper.hpp"
#include "object_properties.hpp"
#include "world_info_interface.hpp"
#include <cmath>
#include <memory>

#include <iostream>

ServerUnit::ServerUnit(const UnitInfo& info)
{
    m_info = info;
    // TODO use stored AI information instead of type
    if (m_info.type == "swordman") {
        m_ai = std::make_unique<AiSwordman>();
    } else if (m_info.type == "archer") {
        m_ai = std::make_unique<AiArcher>();
    } else {
        // TODO pass in and use logger
        std::cerr << "Warning: create a unit with unknown ai type\n";
        m_ai = std::make_unique<AiSwordman>();
    }

    m_hp = m_info.hitpoints;
}

ObjectProperties ServerUnit::saveState() const
{
    ObjectProperties props;
    props.ints[jk::unitID] = m_unitID;
    props.ints[jk::playerID] = m_playerID;
    props.floats[jk::positionX] = m_pos.x;
    props.floats[jk::positionY] = m_pos.y;
    props.floats[jk::hpCurrent] = m_hp;
    props.floats[jk::hpMax] = m_info.hitpoints;
    props.strings[jk::unitType] = m_info.type;
    if (!m_newAnim.empty()) {
        props.strings[jk::unitAnim] = m_newAnim;
        m_newAnim = "";
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
    m_ai->update(elapsed, *this, world);
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
UnitInfo const& ServerUnit::getInfo() const { return m_info; }
