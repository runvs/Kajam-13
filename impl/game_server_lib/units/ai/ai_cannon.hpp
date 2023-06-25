#ifndef JAMTEMPLATE_AI_CANNON_HPP
#define JAMTEMPLATE_AI_CANNON_HPP

#include <units/ai/ai_interface.hpp>

class AiCannon : public AiInterface {
public:
    void update(float elapsed, ServerUnit* unit, WorldInfoInterface& world) override;

private:
    float m_attackTimer { 0.0f };
};

#endif // JAMTEMPLATE_AI_CANNON_HPP
