#include "placed_unit.hpp"
#include <drawable_helpers.hpp>
#include <json_keys.hpp>

PlacedUnit::PlacedUnit(UnitInfo info)
    : m_info { info }
{
}

void PlacedUnit::doCreate()
{
    // TODO use the correct animation instead of a shape
    m_shape = jt::dh::createShapeRect(
        jt::Vector2f { m_info.colliderRadius * 2.0f, m_info.colliderRadius * 2.0f },
        jt::colors::Black, textureManager());
}
void PlacedUnit::doUpdate(const float elapsed) { m_shape->update(elapsed); }

void PlacedUnit::doDraw() const { m_shape->draw(renderTarget()); }

ObjectProperties PlacedUnit::saveState() const
{
    ObjectProperties props;
    props.ints[jk::unitID] = m_unitID;
    props.ints[jk::playerID] = m_playerID;
    props.floats[jk::positionX] = m_shape->getPosition().x;
    props.floats[jk::positionY] = m_shape->getPosition().y;
    props.strings[jk::unitType] = m_info.type;
    return props;
}

void PlacedUnit::setPosition(const jt::Vector2f& pos) { m_shape->setPosition(pos); }

void PlacedUnit::setIDs(int uid, int pid)
{
    m_unitID = uid;
    m_playerID = pid;
}
