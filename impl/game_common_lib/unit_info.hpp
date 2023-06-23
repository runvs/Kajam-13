#ifndef JAMTEMPLATE_UNIT_INFO_HPP
#define JAMTEMPLATE_UNIT_INFO_HPP

#include <nlohmann.hpp>
#include <sprite.hpp>
#include <memory>
#include <string>
#include <vector>

struct AnimationInfo {
    std::string jsonfilename { "" };
};

void to_json(nlohmann::json& j, const AnimationInfo& p);
void from_json(const nlohmann::json& j, AnimationInfo& p);

struct AiInfo {
    enum AiType { CLOSE_COMBAT, RANGED_COMBAT, CANNON };
    AiType type;
    float range { -1.0f };
    float arrowHeight { 0.0f };
    float arrowSpeed { 0.0f };
};

void to_json(nlohmann::json& j, const AiInfo& p);
void from_json(const nlohmann::json& j, AiInfo& p);

struct UpgradeValue {
    float factor { 0.0f };
    float add { 0.0f };
};

struct UpgradeInfo {
    std::string name;
    std::string iconPath;
    std::shared_ptr<jt::Sprite> icon { nullptr };
    int upgradeCost { 0 };

    UpgradeValue damage;
    UpgradeValue attackSpeed;
    UpgradeValue movementSpeed;
    UpgradeValue range;
    UpgradeValue hitpoints;
    UpgradeValue cost;
};

/// Storage for initial values for units
struct UnitInfo {
    std::string type = "";

    int experienceRequiredForLevelUp { 0 };
    int experienceGainWhenKilled { 0 };

    float hitpointsMax { 0.0f };

    // TODO use damage struct
    float damage { 0.0f };

    float attackTimerMax { 0.0f };
    float movementSpeed { 0.0f };

    float colliderRadius { 1.0f };

    int cost { 0 };
    int unlockCost { 0 };

    std::vector<AnimationInfo> animations {};
    AiInfo ai;
    std::vector<UpgradeInfo> possibleUpgrades {};
};

void to_json(nlohmann::json& j, const UpgradeInfo& p);
void from_json(const nlohmann::json& j, UpgradeInfo& p);

void to_json(nlohmann::json& j, const UpgradeValue& p);
void from_json(const nlohmann::json& j, UpgradeValue& p);

void to_json(nlohmann::json& j, const UnitInfo& p);
void from_json(const nlohmann::json& j, UnitInfo& p);

void applyUpgrade(UnitInfo& info, UpgradeInfo const& upg);

UnitInfo getUnitInfoWithLevelAndUpgrades(
    const UnitInfo& baseInfo, int level, std::vector<UpgradeInfo> const& upgrades);

#endif // JAMTEMPLATE_UNIT_INFO_HPP
