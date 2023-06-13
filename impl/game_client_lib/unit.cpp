#include "unit.hpp"
#include <animation.hpp>
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <json_keys.hpp>
#include <object_properties.hpp>
#include <vector.hpp>

Unit::Unit(const UnitInfo &info)
        : m_info{info} {
    m_hpMax = m_info.hitpoints;
}

void Unit::doCreate() {
    m_anim = std::make_shared<jt::Animation>();
    m_anim->loadFromJson(m_info.animations.begin()->jsonfilename, textureManager());
    m_anim->play("idle");
    m_anim->setLooping("death", false);
    m_anim->setOffset(GP::UnitAnimationOffset());
    m_hpBar = std::make_shared<jt::Bar>(16.0f, 4.0f, true, textureManager());
    m_hpBar->setMaxValue(m_hpMax);
    m_hpBar->setBackColor(jt::colors::Gray);
    m_hpBar->setFrontColor(jt::colors::Green);
    m_hpBar->setZ(GP::ZLayerUI());
}

void Unit::doUpdate(float const elapsed) {
    m_anim->update(elapsed);

    if (m_hp > 0) {
        if (m_animTimeUntilBackToIdle != -1.0f) {
            m_animTimeUntilBackToIdle -= elapsed;
            if (m_animTimeUntilBackToIdle <= 0) {
                m_animTimeUntilBackToIdle = -1.0f;
                m_anim->play("idle");
            }
        }
    }

    m_hpBar->setCurrentValue(m_hp);
    m_hpBar->setPosition(m_anim->getPosition() + jt::Vector2f{0.0f, -6.0f});
    m_hpBar->update(elapsed);
}

void Unit::doDraw() const {
    m_anim->draw(renderTarget());
    if (m_hp > 0 && m_hp < m_hpMax) {
        m_hpBar->draw(renderTarget());
    }
}

void Unit::updateState(ObjectProperties const &props) {
    if (props.ints.at(jk::unitID) != m_unitID) {
        getGame()->logger().error(
                "updateState called with invalid unit id", {"Unit", "updateState"});
        return;
    }
    setOffset({props.floats.at(jk::offsetX), props.floats.at(jk::offsetY)});
    setPosition({props.floats.at(jk::positionX), props.floats.at(jk::positionY)});
    if (props.ints.at(jk::playerID) == 0) {
        m_anim->setColor(GP::ColorPlayer0());
    } else {
        m_anim->setColor(GP::ColorPlayer1());
    }
    m_hp = props.floats.at(jk::hpCurrent);

    if (props.strings.count(jk::unitAnim) == 1) {
        auto const animName = props.strings.at(jk::unitAnim);
        m_anim->play(props.strings.at(jk::unitAnim));
        if (animName == "damage") {
            m_animTimeUntilBackToIdle = m_anim->getCurrentAnimTotalTime();
        }
    }

    if (props.bools.at(jk::unitWalkingRight)) {
        m_anim->setOffset(GP::UnitAnimationOffset());
        m_anim->setScale(jt::Vector2f{1.0f, 1.0f});
    } else {
        m_anim->setScale(jt::Vector2f{-1.0f, 1.0f});
        m_anim->setOffset(GP::UnitAnimationOffset() + jt::Vector2f{32.0f, 0.0f});
    }
}

void Unit::setPosition(jt::Vector2f const &pos) {
    m_position = pos;
    m_anim->setPosition(m_position + m_offset);
}

jt::Vector2f Unit::getPosition() const { return m_position; }

void Unit::setOffset(jt::Vector2f const &offset) { m_offset = offset; }

jt::Vector2f Unit::getOffset() const { return m_offset; }

ObjectProperties Unit::saveState() const {
    ObjectProperties props;
    props.ints[jk::unitID] = m_unitID;
    props.ints[jk::playerID] = m_playerID;
    props.floats[jk::positionX] = m_position.x;
    props.floats[jk::positionY] = m_position.y;
    props.floats[jk::offsetX] = m_offset.x;
    props.floats[jk::offsetY] = m_offset.y;
    return props;
}

int Unit::getUnitID() const { return m_unitID; }

void Unit::setIDs(int uid, int pid) {
    m_unitID = uid;
    m_playerID = pid;
}

int Unit::getPlayerID() const { return m_playerID; }

bool Unit::isUnitAlive() const {
    return m_hp > 0;
}
