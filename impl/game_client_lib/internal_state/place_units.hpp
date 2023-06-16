#ifndef JAMTEMPLATE_PLACE_UNITS_HPP
#define JAMTEMPLATE_PLACE_UNITS_HPP

#include "internal_state_interface.hpp"
#include <unit.hpp>
#include <memory>

class PlaceUnits : public InternalStateInterface {
public:
    void update(StateGame& state, float elapsed) override;
    void draw(StateGame& state) override;

private:
    std::shared_ptr<Unit> m_selectedUnit { nullptr };
};

#endif // JAMTEMPLATE_PLACE_UNITS_HPP
