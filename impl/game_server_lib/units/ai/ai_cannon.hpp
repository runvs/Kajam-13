#ifndef JAMTEMPLATE_AI_CANNON_HPP
#define JAMTEMPLATE_AI_CANNON_HPP

#include <units/ai/ai_interface.hpp>

class AiCannon : public AiInterface {
public:
    AiCannon();
    void update(float elapsed, ServerUnit* unit, WorldInfoInterface& world) override;

private:
    float m_attackTimer;
};

#endif // JAMTEMPLATE_AI_CANNON_HPP
