#include "unit.hpp"
#include "audio/sound/sound_group.hpp"
#include <animation.hpp>
#include <color/color.hpp>
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <math_helper.hpp>
#include <rect.hpp>
#include <vector.hpp>

Unit::Unit(const UnitInfo& info)
    : m_info { info }
{
    m_hpMax = m_info.hitpointsMax;
}

void Unit::doCreate()
{
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

    m_levelText = jt::dh::createText(renderTarget(), "", 12);
    m_levelText->setZ(GP::ZLayerUI());

    m_sfxCloseCombat1 = getGame()->audio().addTemporarySound("assets/sfx/cc1.wav");
    m_sfxCloseCombat2 = getGame()->audio().addTemporarySound("assets/sfx/cc2.wav");
    m_sfxCloseCombat3 = getGame()->audio().addTemporarySound("assets/sfx/cc3.wav");

    m_sfxCloseCombat1->setVolume(0.5f);
    m_sfxCloseCombat2->setVolume(0.5f);
    m_sfxCloseCombat3->setVolume(0.5f);
    m_sfxGrpCloseCombat = getGame()->audio().addTemporarySoundGroup(
        { m_sfxCloseCombat1, m_sfxCloseCombat2, m_sfxCloseCombat3 });

    m_sfxArcher = getGame()->audio().addTemporarySound("assets/sfx/archer.wav");
    m_sfxCrossbow = getGame()->audio().addTemporarySound("assets/sfx/crossbow.wav");
}

void Unit::doUpdate(float const elapsed)
{
    m_anim->update(elapsed);
    m_levelText->setPosition(m_anim->getPosition() + jt::Vector2f { -4.0f, -6.0f });
    m_levelText->update(elapsed);

    playAnimation();
    if (isUnitAlive()) {
        if (m_animTimeUntilBackToIdle != -1.0f) {
            m_animTimeUntilBackToIdle -= elapsed;
            if (m_animTimeUntilBackToIdle <= 0) {
                m_animTimeUntilBackToIdle = -1.0f;
                m_anim->play("idle");
            }
        }
    }

    for (auto& kvp : m_soundsToPlay) {
        kvp.first -= elapsed;
        if (kvp.first <= 0) {
            kvp.second->play();
        }
    }
    (void)std::erase_if(m_soundsToPlay, [](auto const& kvp) { return kvp.first <= 0; });

    m_hpBar->setCurrentValue(m_hp);
    m_hpBar->setPosition(m_anim->getPosition() + jt::Vector2f { 0.0f, -6.0f });
    m_hpBar->update(elapsed);
}

void Unit::doDraw() const
{
    m_anim->draw(renderTarget());
    if (m_hp > 0 && m_hp < m_hpMax) {
        m_hpBar->draw(renderTarget());
    }
    if (m_hp > 0) {
        m_levelText->draw(renderTarget());
    }
}

void Unit::updateState(UnitServerToClientData const& props)
{
    if (props.unitID != m_unitID) {
        getGame()->logger().error(
            "updateState called with invalid unit id", { "Unit", "updateState" });
        return;
    }
    setOffset({ props.offsetX, props.offsetY });
    setPosition({ props.positionX, props.positionY });
    if (props.playerID == 0) {
        m_anim->setColor(GP::ColorPlayer0());
    } else {
        m_anim->setColor(GP::ColorPlayer1());
    }
    m_hp = props.hpCurrent;

    if (props.unitAnim.has_value()) {
        m_newAnimName = props.unitAnim.value();
    }

    if (props.unitWalkingRight) {
        m_anim->setOffset(GP::UnitAnimationOffset());
        m_anim->setScale(jt::Vector2f { 1.0f, 1.0f });
    } else {
        m_anim->setScale(jt::Vector2f { -1.0f, 1.0f });
        m_anim->setOffset(GP::UnitAnimationOffset() + jt::Vector2f { 32.0f, 0.0f });
    }
    if (props.level != 1) {

        m_level = props.level;
        if (m_level != 1) {
            m_levelText->setText(std::to_string(m_level));
        }
    }
}

void Unit::playAnimation()
{
    if (m_newAnimName == "") {
        return;
    }
    auto const newAnimName = m_newAnimName;
    m_newAnimName = "";

    if (newAnimName != "idle") {
        m_animTimeUntilBackToIdle = m_anim->getAnimTotalTime(newAnimName);
    }
    if (newAnimName == "damage" || newAnimName == "death") {
        m_anim->flash(0.15f, jt::colors::Red);
    }
    if (newAnimName == "attack") {
        if (m_info.type == "swordman" || m_info.type == "shieldman" || m_info.type == "horseman") {
            m_soundsToPlay.push_back(std::make_pair(0.4f, m_sfxGrpCloseCombat));
        } else if (m_info.type == "archer") {
            m_soundsToPlay.push_back(std::make_pair(0.81f, m_sfxArcher));
        } else if (m_info.type == "crossbow") {
            m_soundsToPlay.push_back(std::make_pair(0.81f, m_sfxCrossbow));
        }
    }
    auto const currentAnimationName = m_anim->getCurrentAnimationName();

    auto const currentPriority = GP::animationPriorities().at(currentAnimationName);
    auto const newPriority = GP::animationPriorities().at(newAnimName);

    if (newPriority < currentPriority) {
        return;
    }

    bool const forceRestart = (newAnimName != "walk");

    m_anim->play(newAnimName, 0, forceRestart);
    m_anim->update(0.0f);
}

void Unit::setPosition(jt::Vector2f const& pos)
{
    m_position = pos;
    m_anim->setPosition(m_position + m_offset);
}

jt::Vector2f Unit::getPosition() const { return m_position; }

void Unit::setOffset(jt::Vector2f const& offset) { m_offset = offset; }

jt::Vector2f Unit::getOffset() const { return m_offset; }

int Unit::getUnitID() const { return m_unitID; }

void Unit::setIDs(int uid, int pid)
{
    m_unitID = uid;
    m_playerID = pid;
}

int Unit::getPlayerID() const { return m_playerID; }

bool Unit::isUnitAlive() const { return m_hp > 0; }
bool Unit::isMouseOver() const
{
    auto const mp = getGame()->input().mouse()->getMousePositionWorld();
    jt::Rectf const rect { m_position.x + m_offset.x, m_position.y + m_offset.y, 16.0f, 16.0f };

    return jt::MathHelper::checkIsIn(rect, mp);
}
UnitInfo const& Unit::getInfo() const { return m_info; }

int Unit::getLevel() const { return m_level; }

void Unit::resetForNewRound()
{
    m_anim->play("idle", 0, true);
    m_animTimeUntilBackToIdle = -1.0f;
}
void Unit::flash() { m_anim->flash(0.4f, jt::colors::Green); }
