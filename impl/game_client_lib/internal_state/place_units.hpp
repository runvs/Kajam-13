#ifndef JAMTEMPLATE_PLACE_UNITS_HPP
#define JAMTEMPLATE_PLACE_UNITS_HPP

#include <internal_state/internal_state_interface.hpp>
#include <sprite.hpp>
#include <state_game.hpp>
#include <unit.hpp>
#include <unit_placement/placed_unit.hpp>
#include <memory>

class PlaceUnits : public InternalStateInterface {
public:
    void update(StateGame& state, float elapsed) override;
    void draw(StateGame& state) override;

private:
    // TODO extract a common interface
    std::shared_ptr<jt::Sprite> m_imageEndPlacement { nullptr };
    std::shared_ptr<Unit> m_selectedUnit { nullptr };
    std::shared_ptr<PlacedUnit> m_selectedPlacedUnit { nullptr };
    std::string m_selectedUnitType {};
};

#endif // JAMTEMPLATE_PLACE_UNITS_HPP
