#ifndef JAMTEMPLATE_CRITTER_HPP
#define JAMTEMPLATE_CRITTER_HPP

#include <animation.hpp>
#include <game_object.hpp>
#include <memory>
#include <string>

class Critter : public jt::GameObject {
    std::shared_ptr<jt::Animation> m_animation;
    float m_animSpeedFactor;
    jt::Vector2f m_initialPos;
    bool m_isMoving { false };
    jt::Vector2f m_movingDir { 0.0f, 0.0f };
    std::string const m_assetName;
    std::string const m_movingAnimName;

public:
    void setPosition(jt::Vector2f const& pos);

    void setClosestUnitPosition(jt::Vector2f const& unitPos);

    void resetOnRoundStart();

    jt::Vector2f getPosition() const;
    bool isMoving() const;

protected:
    Critter(std::string const assetName, std::string const movingAnimName);

private:
    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;
};

#endif // JAMTEMPLATE_CRITTER_HPP
