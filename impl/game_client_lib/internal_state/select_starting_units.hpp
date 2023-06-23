#ifndef JAMTEMPLATE_SELECT_STARTING_UNITS_HPP
#define JAMTEMPLATE_SELECT_STARTING_UNITS_HPP

#include <internal_state/internal_state_interface.hpp>

class SelectStartingUnits : public InternalStateInterface {
public:
    void update(StateGame& state, float elapsed) override;
    void draw(StateGame& state) override;
};

#endif // JAMTEMPLATE_SELECT_STARTING_UNITS_HPP
