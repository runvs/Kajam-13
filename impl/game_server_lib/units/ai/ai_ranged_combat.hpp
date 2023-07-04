#ifndef JAMTEMPLATE_AI_RANGED_COMBAT_HPP
#define JAMTEMPLATE_AI_RANGED_COMBAT_HPP

#include <units/ai/ai_interface.hpp>

class AiRangedCombat : public AiInterface {
public:
    AiRangedCombat();
    void update(float elapsed, ServerUnit* unit, WorldInfoInterface& world) override;

private:
    float m_attackTimer;
};

#endif // JAMTEMPLATE_AI_RANGED_COMBAT_HPP
