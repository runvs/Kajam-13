#include "unit.hpp"
#include "drawable_helpers.hpp"
#include "object_properties.hpp"
#include "vector.hpp"
#include <game_interface.hpp>
#include <json_keys.hpp>

Unit::Unit() { m_unitID = jt::CountedObj<Unit>::createdObjects(); }

void Unit::doCreate()
{
    m_shape = jt::dh::createShapeRect(
        jt::Vector2f { 16.0f, 16.0f }, jt::colors::Black, textureManager());
}

void Unit::doUpdate(float const elapsed) { m_shape->update(elapsed); }

void Unit::doDraw() const { m_shape->draw(renderTarget()); }

void Unit::updateState(ObjectProperties const& props)
{
    if (props.ints.at(jk::unitID) != m_unitID) {
        getGame()->logger().error(
            "updateState called with invalid unit id", { "Unit", "updateState" });
        return;
    }
    m_shape->setPosition({ props.floats.at(jk::positionX), props.floats.at(jk::positionY) });
}

void Unit::setPosition(jt::Vector2f const& pos) { m_shape->setPosition(pos); }

ObjectProperties Unit::saveState() const
{
    ObjectProperties props;
    props.ints[jk::unitID] = m_unitID;
    props.ints[jk::playerID] = m_playerID;
    props.floats[jk::positionX] = m_shape->getPosition().x;
    props.floats[jk::positionY] = m_shape->getPosition().y;
    return props;
}

int Unit::getUnitID() const { return m_unitID; }
void Unit::setPlayerID(int pid) { m_playerID = pid; }
