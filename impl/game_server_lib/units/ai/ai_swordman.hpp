#ifndef JAMTEMPLATE_AI_SWORDMAN_HPP
#define JAMTEMPLATE_AI_SWORDMAN_HPP

#include <units/ai/ai_interface.hpp>

class AiSwordman : public AiInterface {
public:
    void update(float elapsed, ServerUnit& unit, WorldInfoInterface& world) override;

private:
    float m_attackTimer { 0.0f };
};

#endif // JAMTEMPLATE_AI_SWORDMAN_HPP
