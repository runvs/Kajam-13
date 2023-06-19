#ifndef JAMTEMPLATE_ARROW_INFO_HPP
#define JAMTEMPLATE_ARROW_INFO_HPP

#include "damage_info.hpp"
#include "vector.hpp"
#include <nlohmann.hpp>

struct ArrowInfo {
    jt::Vector2f startPos { 0.0f, 0.0f };
    jt::Vector2f endPos { 0.0f, 0.0f };
    jt::Vector2f currentPos { 0.0f, 0.0f };
    DamageInfo damage { 0.0f };
    float age { 0.0f };
    float totalTime { 0.0f };
    int targetPlayerId { -1 };
    float maxHeight { 32.0f };
    int shooterPlayerId { -1 };
    int shooterUnitId { 0 };
};

void to_json(nlohmann::json& j, const ArrowInfo& data);
void from_json(const nlohmann::json& j, ArrowInfo& data);

#endif // JAMTEMPLATE_ARROW_INFO_HPP
