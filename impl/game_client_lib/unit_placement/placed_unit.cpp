#include "placed_unit.hpp"
#include "vector.hpp"
#include <drawable_helpers.hpp>
#include <game_properties.hpp>
#include <json_keys.hpp>
#include <memory>

PlacedUnit::PlacedUnit(UnitInfo info)
    : m_info { info }
{
}

void PlacedUnit::doCreate()
{
    m_anim = std::make_shared<jt::Animation>();
    m_anim->loadFromJson(m_info.animations.begin()->jsonfilename, textureManager());
    m_anim->play("idle");
    m_anim->setOffset(GP::UnitAnimationOffset());
}
void PlacedUnit::doUpdate(const float elapsed) { m_anim->update(elapsed); }

void PlacedUnit::doDraw() const { m_anim->draw(renderTarget()); }

ObjectProperties PlacedUnit::saveState() const
{
    ObjectProperties props;
    props.ints[jk::unitID] = m_unitID;
    props.ints[jk::playerID] = m_playerID;
    props.floats[jk::positionX] = m_anim->getPosition().x;
    props.floats[jk::positionY] = m_anim->getPosition().y;
    props.strings[jk::unitType] = m_info.type;
    return props;
}

void PlacedUnit::setPosition(const jt::Vector2f& pos) { m_anim->setPosition(pos); }

jt::Vector2f PlacedUnit::getPosition() const { return m_anim->getPosition(); }

void PlacedUnit::setIDs(int uid, int pid)
{
    m_unitID = uid;
    m_playerID = pid;
}
