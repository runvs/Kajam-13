#ifndef JAMTEMPLATE_END_WIN_HPP
#define JAMTEMPLATE_END_WIN_HPP

#include "internal_state_interface.hpp"

class EndWin : public InternalStateInterface {
public:
    void update(StateGame& state, float elapsed) override;
    void draw(StateGame& state) override;
};

#endif // JAMTEMPLATE_END_WIN_HPP
