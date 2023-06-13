#ifndef JAMTEMPLATE_UNIT_INFO_HPP
#define JAMTEMPLATE_UNIT_INFO_HPP

#include <nlohmann.hpp>
#include <vector.hpp>
#include <string>
#include <vector>

struct AnimationInfo {

    std::string jsonfilename { "" };
};

struct AiInfo {
    enum AiType { CLOSE_COMBAT, RANGED_COMBAT };
    AiType type;
    float range { -1.0f };
    float arrowHeight { 0.0f };
    float arrowSpeed { 0.0f };
};

struct UnitInfo {
    std::string type = "";

    int experience { 0 };
    int experienceForLevelUp { 100 };
    
    float hitpoints { 0.0f };

    // TODO use damage struct
    float damage { 0.0f };

    float attackTimerMax { 0.0f };
    float movementSpeed { 0.0f };

    float colliderRadius { 1.0f };

    int cost { 0 };
    int unlockCost { 0 };

    std::vector<AnimationInfo> animations {};
    AiInfo ai;
};

void to_json(nlohmann::json& j, const AnimationInfo& p);
void to_json(nlohmann::json& j, const AiInfo& p);
void to_json(nlohmann::json& j, const UnitInfo& p);

void from_json(const nlohmann::json& j, AnimationInfo& p);
void from_json(const nlohmann::json& j, AiInfo& p);
void from_json(const nlohmann::json& j, UnitInfo& p);

#endif // JAMTEMPLATE_UNIT_INFO_HPP
