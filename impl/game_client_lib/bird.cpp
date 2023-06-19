#include "bird.hpp"
#include <game_interface.hpp>
#include <random/random.hpp>

void Bird::doCreate()
{
    m_animation = std::make_shared<jt::Animation>();
    m_animation->loadFromJson("assets/bird.json", textureManager());

    auto const numberOfFrames = m_animation->getNumberOfFramesInAnimation("idle");
    m_animation->play("idle", jt::Random::getInt(0, numberOfFrames - 1), true);

    m_animSpeedFactor = jt::Random::getFloatGauss(1.0f, 0.05f);
}

void Bird::doUpdate(const float elapsed) { m_animation->update(elapsed * m_animSpeedFactor); }

void Bird::doDraw() const { m_animation->draw(renderTarget()); }
void Bird::setPosition(jt::Vector2f const& pos) { m_animation->setPosition(pos); }
