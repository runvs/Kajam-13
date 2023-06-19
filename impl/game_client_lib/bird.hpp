#ifndef JAMTEMPLATE_BIRD_HPP
#define JAMTEMPLATE_BIRD_HPP

#include <animation.hpp>
#include <game_object.hpp>

class Bird : public jt::GameObject {
public:
    void setPosition(jt::Vector2f const& pos);

private:
    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;

    std::shared_ptr<jt::Animation> m_animation;
    float m_animSpeedFactor;
};

#endif // JAMTEMPLATE_BIRD_HPP
