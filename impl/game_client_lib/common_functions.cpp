#include "common_functions.hpp"
#include <game_properties.hpp>

void turnUnitIntoDirection(std::shared_ptr<jt::DrawableInterface> drawable, bool lookingRight)
{
    if (lookingRight) {
        drawable->setOffset(GP::UnitAnimationOffset());
        drawable->setScale(jt::Vector2f { 1.0f, 1.0f });
    } else {
        drawable->setScale(jt::Vector2f { -1.0f, 1.0f });
        drawable->setOffset(GP::UnitAnimationOffset() + jt::Vector2f { 32.0f, 0.0f });
    }
}
