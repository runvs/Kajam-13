#include "unit.hpp"
#include "drawable_helpers.hpp"
#include "object_properties.hpp"
#include "vector.hpp"

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
    m_shape->setPosition({ props.floats.at("posX"), props.floats.at("posY") });
}

void Unit::setPosition(jt::Vector2f const& pos) { m_shape->setPosition(pos); }

ObjectProperties Unit::saveState() const
{
    ObjectProperties props;
    props.ints["unitID"] = m_unitID;
    props.floats["posX"] = m_shape->getPosition().x;
    props.floats["posY"] = m_shape->getPosition().y;
    return props;
}
int Unit::getUnitID() const { return m_unitID; }
