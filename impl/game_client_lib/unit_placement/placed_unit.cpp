#include "placed_unit.hpp"
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <math_helper.hpp>
#include <network_data/unit_client_to_server_data.hpp>
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

UnitClientToServerData PlacedUnit::saveState() const
{
    UnitClientToServerData unitData;
    unitData.unitID = m_unitID;
    unitData.playerID = m_playerID;
    unitData.positionX = m_position.x;
    unitData.positionY = m_position.y;
    unitData.offsetX = m_offset.x;
    unitData.offsetY = m_offset.y;
    unitData.unitType = m_info.type;
    return unitData;
}

void PlacedUnit::setPosition(const jt::Vector2f& pos)
{
    m_position = pos;
    m_anim->setPosition(m_position + m_offset);
}

jt::Vector2f PlacedUnit::getPosition() const { return m_position; }

void PlacedUnit::setOffset(const jt::Vector2f& offset) { m_offset = offset; }

jt::Vector2f PlacedUnit::getOffset() const { return m_offset; }

void PlacedUnit::setIDs(int uid, int pid)
{
    m_unitID = uid;
    m_playerID = pid;
}

bool PlacedUnit::isMouseOver() const
{
    auto const mp = getGame()->input().mouse()->getMousePositionWorld();
    jt::Rectf const rect { m_position.x + m_offset.x, m_position.y + m_offset.y, 16.0f, 16.0f };

    return jt::MathHelper::checkIsIn(rect, mp);
}

UnitInfo const& PlacedUnit::getInfo() const { return m_info; }
