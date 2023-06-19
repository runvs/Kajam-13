#include "bird.hpp"
#include <game_interface.hpp>
#include <math_helper.hpp>
#include <random/random.hpp>

void Bird::doCreate()
{
    m_animation = std::make_shared<jt::Animation>();
    m_animation->loadFromJson("assets/bird.json", textureManager());

    auto const numberOfFrames = m_animation->getNumberOfFramesInAnimation("idle");
    m_animation->play("idle", jt::Random::getInt(0, numberOfFrames - 1), true);

    m_animSpeedFactor = jt::Random::getFloatGauss(1.0f, 0.05f);
}

void Bird::doUpdate(const float elapsed)
{
    m_animation->update(elapsed * m_animSpeedFactor);

    if (m_isFlying) {
        auto const pos = m_animation->getPosition() + elapsed * m_flyingDir;
        m_animation->setPosition(pos);
    }
}

void Bird::doDraw() const { m_animation->draw(renderTarget()); }

void Bird::setPosition(jt::Vector2f const& pos)
{
    m_animation->setPosition(pos);
    m_initialPos = pos;
}

void Bird::resetOnRoundStart()
{
    getGame()->logger().warning("reset bird");
    m_animation->setPosition(m_initialPos);

    auto const numberOfFrames = m_animation->getNumberOfFramesInAnimation("idle");
    m_animation->play("idle", jt::Random::getInt(0, numberOfFrames - 1), true);
    m_isFlying = false;
}

void Bird::setClosestUnitPosition(const jt::Vector2f& unitPos)
{
    if (m_isFlying) {
        return;
    }

    auto const dir = m_initialPos - unitPos;
    auto const dist = jt::MathHelper::length(dir);
    if (dist < 32) {
        m_isFlying = true;
        m_flyingDir = jt::MathHelper::normalized(dir) * 40.0f;
        m_animation->play("fly");
    }
}

jt::Vector2f Bird::getPosition() const { return m_initialPos; }

bool Bird::isFlying() const { return m_isFlying; }
