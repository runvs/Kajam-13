#include "unit.hpp"
#include "drawable_helpers.hpp"
#include "vector.hpp"

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
jt::Vector2f Unit::getPosition() const { return m_shape->getPosition(); }
void Unit::setPosition(jt::Vector2f const& pos) { m_shape->setPosition(pos); }
