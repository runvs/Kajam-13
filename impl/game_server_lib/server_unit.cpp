#include "server_unit.hpp"
#include <object_properties.hpp>
#include <cmath>

ObjectProperties ServerUnit::saveState() const
{
    ObjectProperties props;
    props.floats["posX"] = m_pos.x;
    props.floats["posY"] = m_pos.y;
    return props;
}

void ServerUnit::update(float elapsed)
{
    float const speed = 16.0f;
    m_pos.x += elapsed * speed;

    m_age += elapsed;
    m_pos.y += elapsed * 40 * cos(m_age * 2);
}

void ServerUnit::setPosition(jt::Vector2f const& pos) { m_pos = pos; }

jt::Vector2f ServerUnit::getPosition() const { return m_pos; }
