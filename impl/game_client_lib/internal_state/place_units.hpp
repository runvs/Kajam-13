#ifndef JAMTEMPLATE_PLACE_UNITS_HPP
#define JAMTEMPLATE_PLACE_UNITS_HPP

#include <internal_state/internal_state_interface.hpp>
#include <shape.hpp>
#include <sprite.hpp>
#include <state_game.hpp>
#include <memory>

class UnitInterface;

class PlaceUnits : public InternalStateInterface {
public:
    void create(StateGame& state) override;
    void update(StateGame& state, float elapsed) override;
    void draw(StateGame& state) override;

private:
    std::shared_ptr<jt::Sprite> m_imageEndPlacement { nullptr };
    std::shared_ptr<jt::Shape> m_rangeIndicator { nullptr };

    std::shared_ptr<UnitInterface> m_unitInterfaceSelected { nullptr };

    std::string m_selectedUnitType {};

    mutable bool m_deleteUnitOnclick { false };

    void drawRangeIndicator(StateGame& state);

    void drawUnitUpgradeWindow(std::shared_ptr<UnitInterface> selectedUnit, StateGame& state);
};

#endif // JAMTEMPLATE_PLACE_UNITS_HPP
