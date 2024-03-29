#include "bunny.hpp"
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <math_helper.hpp>
#include <random/random.hpp>

Critter::Critter(std::string const& assetName, std::string const& movingAnimName)
    : m_assetName { assetName }
    , m_movingAnimName { movingAnimName }
{
}

void Critter::doCreate()
{
    m_animation = std::make_shared<jt::Animation>();
    m_animation->loadFromAseprite(m_assetName.c_str(), textureManager());

    auto const numberOfFrames = m_animation->getNumberOfFramesInAnimation("idle");
    m_animation->play("idle", jt::Random::getInt(0, static_cast<int>(numberOfFrames) - 1), true);

    m_animSpeedFactor = jt::Random::getFloatGauss(1.0f, 0.05f);

    // random looking direction
    if (jt::Random::getInt(0, 1) == 0) {
        m_animation->setScale(jt::Vector2f { 1.0f, 1.0f });
        m_animation->setOffset(GP::UnitAnimationOffset());
    } else {
        m_animation->setScale(jt::Vector2f { -1.0f, 1.0f });
        m_animation->setOffset(GP::UnitAnimationOffset() + jt::Vector2f { 32.0f, 0.0f });
    }
    m_animation->setPosition(m_initialPos);
}

void Critter::doUpdate(float const elapsed)
{
    m_animation->update(elapsed * m_animSpeedFactor);

    if (m_isMoving) {
        auto const pos = m_animation->getPosition() + elapsed * m_movingDir;
        m_animation->setPosition(pos);
    }
}

void Critter::doDraw() const { m_animation->draw(renderTarget()); }

void Critter::setPosition(jt::Vector2f const& pos) { m_initialPos = pos; }

void Critter::resetForNewRound()
{
    m_animation->setPosition(m_initialPos);

    auto const numberOfFrames = m_animation->getNumberOfFramesInAnimation("idle");
    m_animation->play("idle", jt::Random::getInt(0, static_cast<int>(numberOfFrames) - 1), true);
    m_isMoving = false;
}

void Critter::setClosestUnitPosition(jt::Vector2f const& unitPos)
{
    if (m_isMoving) {
        return;
    }

    auto const dir = m_initialPos - unitPos;
    auto const dist = jt::MathHelper::length(dir);
    if (dist < 32) {
        m_isMoving = true;
        m_movingDir = jt::MathHelper::normalized(dir) * 40.0f;
        if (m_movingDir.x > 0) {
            m_animation->setScale(jt::Vector2f { 1.0f, 1.0f });
            m_animation->setOffset(GP::UnitAnimationOffset());
        } else {
            m_animation->setScale(jt::Vector2f { -1.0f, 1.0f });
            m_animation->setOffset(GP::UnitAnimationOffset() + jt::Vector2f { 32.0f, 0.0f });
        }
        m_animation->play(m_movingAnimName.c_str());
    }
}

jt::Vector2f Critter::getPosition() const { return m_initialPos; }

bool Critter::isMoving() const { return m_isMoving; }
