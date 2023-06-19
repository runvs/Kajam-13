#ifndef JAMTEMPLATE_BIRD_HPP
#define JAMTEMPLATE_BIRD_HPP

#include <animation.hpp>
#include <game_object.hpp>

class Bird : public jt::GameObject {
public:
    void setPosition(jt::Vector2f const& pos);

    void setClosestUnitPosition(jt::Vector2f const& unitPos);

    void resetOnRoundStart();

    jt::Vector2f getPosition() const;
    bool isFlying() const;

private:
    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;

    std::shared_ptr<jt::Animation> m_animation;
    float m_animSpeedFactor;

    jt::Vector2f m_initialPos;

    bool m_isFlying { false };
    jt::Vector2f m_flyingDir { 0.0f, 0.0f };
};

#endif // JAMTEMPLATE_BIRD_HPP
