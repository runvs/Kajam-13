#ifndef JAMTEMPLATE_AI_SHIELDMAGE_HPP
#define JAMTEMPLATE_AI_SHIELDMAGE_HPP

#include <units/ai/ai_interface.hpp>
#include <vector2.hpp>

class AiShieldmage : public AiInterface {
public:
    void update(float elapsed, ServerUnit* unit, WorldInfoInterface& world) override;

private:
    bool m_shieldTriggerd { false };
    jt::Vector2f m_initialPos;
};

#endif // JAMTEMPLATE_AI_SHIELDMAGE_HPP
