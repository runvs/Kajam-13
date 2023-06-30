#ifndef JAMTEMPLATE_AI_INFO_HPP
#define JAMTEMPLATE_AI_INFO_HPP

#include <nlohmann.hpp>

struct AiInfo {
    enum AiType { CLOSE_COMBAT, RANGED_COMBAT, CANNON };
    AiType type;
    float range { -1.0f };
    float arrowHeight { 0.0f };
    float arrowSpeed { 0.0f };
};

void to_json(nlohmann::json& j, const AiInfo& p);
void from_json(const nlohmann::json& j, AiInfo& p);

#endif // JAMTEMPLATE_AI_INFO_HPP
