#include "common_functions.hpp"
#include <math_helper.hpp>
#include <unit_placement/placed_unit.hpp>

void CommonFunctions::updateBirds(StateGame& state)
{
    for (auto& b : *state.getBirds()) {
        auto bird = b.lock();
        if (bird->isFlying()) {
            continue;
        }
        auto const bp = bird->getPosition();
        jt::Vector2f closestPosition { -999999, -999999 };
        float closestDistance = 9999999;
        if (!state.getUnits()) {
            break;
        }

        auto const lmb = [&bp, &closestDistance, &closestPosition](auto u) {
            auto const unit = u.lock();
            auto const up = unit->getPosition();
            auto const dist = jt::MathHelper::lengthSquared(bp - up);
            if (dist < closestDistance) {
                closestDistance = dist;
                closestPosition = up;
            }
        };

        for (auto& u : *state.getUnits()) {
            lmb(u);
        }
        for (auto& u : *state.getPlacementManager()->getPlacedUnitsGO()) {
            lmb(u);
        }

        bird->setClosestUnitPosition(closestPosition);
    }
}
