#include "server_unit.hpp"
#include <json_keys.hpp>
#include <object_properties.hpp>
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
    // determine target
    auto const target = world.getClosestTargetTo(m_pos, m_playerID);

    float const speed = 2 * m_info.movementSpeed;

    m_age += elapsed;
    m_pos.y += elapsed * 40 * cos(m_age * speed);
    m_hp -= elapsed * 2.0f;
}

void ServerUnit::setPosition(jt::Vector2f const& pos) { m_pos = pos; }

jt::Vector2f ServerUnit::getPosition() const { return m_pos; }
void ServerUnit::setUnitID(int unitID) { m_unitID = unitID; }
int ServerUnit::getPlayerID() const { return m_playerID; }
int ServerUnit::getUnitID() const { return m_unitID; }
