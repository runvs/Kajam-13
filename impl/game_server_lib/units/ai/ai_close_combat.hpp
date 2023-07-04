#ifndef JAMTEMPLATE_AI_CLOSE_COMBAT_HPP
#define JAMTEMPLATE_AI_CLOSE_COMBAT_HPP

#include <units/ai/ai_interface.hpp>

class AiCloseCombat : public AiInterface {
public:
    AiCloseCombat();
    void update(float elapsed, ServerUnit* unit, WorldInfoInterface& world) override;

private:
    float m_attackTimer;
};

#endif // JAMTEMPLATE_AI_CLOSE_COMBAT_HPP
