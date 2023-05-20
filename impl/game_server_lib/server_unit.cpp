#include "server_unit.hpp"
#include <json_keys.hpp>
#include <object_properties.hpp>
#include <cmath>

ObjectProperties ServerUnit::saveState() const
{
    ObjectProperties props;
    props.ints[jk::unitID] = m_unitID;
    props.ints[jk::playerID] = m_playerID;
    props.floats[jk::positionX] = m_pos.x;
    props.floats[jk::positionY] = m_pos.y;

    return props;
}

void ServerUnit::update(float elapsed)
{
    float const speed = 16.0f;
    // TODO use any smart logic here about walking to the closest target and so on.
    if (m_playerID == 0) {
        m_pos.x += elapsed * speed;
    } else {
        m_pos.x -= elapsed * speed;
    }
    m_age += elapsed;
    m_pos.y += elapsed * 40 * cos(m_age * 2);
}

void ServerUnit::setPosition(jt::Vector2f const& pos) { m_pos = pos; }

jt::Vector2f ServerUnit::getPosition() const { return m_pos; }
void ServerUnit::setUnitID(int unitID) { m_unitID = unitID; }

void ServerUnit::updateState(ObjectProperties const& props)
{
    m_unitID = props.ints.at(jk::unitID);
    m_playerID = props.ints.at(jk::playerID);
    m_pos = jt::Vector2f { props.floats.at(jk::positionX), props.floats.at(jk::positionY) };
}
