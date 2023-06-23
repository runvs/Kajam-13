#ifndef JAMTEMPLATE_END_LOSE_HPP
#define JAMTEMPLATE_END_LOSE_HPP

#include <internal_state/internal_state_interface.hpp>

class EndLose : public InternalStateInterface {
public:
    void update(StateGame& state, float elapsed) override;
    void draw(StateGame& state) override;
};

#endif // JAMTEMPLATE_END_LOSE_HPP
