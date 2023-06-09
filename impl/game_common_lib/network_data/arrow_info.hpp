#ifndef JAMTEMPLATE_ARROW_INFO_HPP
#define JAMTEMPLATE_ARROW_INFO_HPP

#include <network_data/damage_info.hpp>
#include <nlohmann.hpp>
#include <vector2.hpp>

struct ArrowInfo {
    jt::Vector2f startPos { 0.0f, 0.0f };
    jt::Vector2f endPos { 0.0f, 0.0f };
    jt::Vector2f currentPos { 0.0f, 0.0f };
    DamageInfo damage { 0.0f, {} };
    float age { 0.0f };
    float totalTime { 0.0f };
    int targetPlayerId { -1 };
    float maxHeight { 32.0f };
    float currentHeight { 0.0f };
    int shooterPlayerId { -1 };
    int shooterUnitId { 0 };
    float splashRadius { 0.0f };
    float arrowScale { 1.0f };
};

void to_json(nlohmann::json& j, const ArrowInfo& data);
void from_json(const nlohmann::json& j, ArrowInfo& data);

#endif // JAMTEMPLATE_ARROW_INFO_HPP
