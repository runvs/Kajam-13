#ifndef JAMTEMPLATE_UNIT_INFO_HPP
#define JAMTEMPLATE_UNIT_INFO_HPP

#include <network_data/ai_info.hpp>
#include <network_data/animation_info.hpp>
#include <network_data/armor_info.hpp>
#include <network_data/damage_info.hpp>
#include <network_data/upgrade_info.hpp>
#include <nlohmann.hpp>
#include <memory>
#include <string>
#include <vector>

/// Storage for initial values for units
struct UnitInfo {
    std::string type { "" };

    std::string description { "" };

    int experienceRequiredForLevelUp { 0 };
    int experienceGainWhenKilled { 0 };

    float hitpointsMax { 0.0f };

    DamageInfo damage { 0.0f, {} };
    ArmorInfo armor {};

    float attackTimerMax { 0.0f };
    float movementSpeed { 0.0f };

    float colliderRadius { 1.0f };

    int cost { 0 };
    int unlockCost { 0 };

    std::vector<AnimationInfo> animations {};
    AiInfo ai;
    std::vector<UpgradeInfo> possibleUpgrades {};
};

void to_json(nlohmann::json& j, const UnitInfo& data);
void from_json(const nlohmann::json& j, UnitInfo& data);

void applyUpgrade(UnitInfo& info, UpgradeInfo const& upg);

UnitInfo getUnitInfoWithLevelAndUpgrades(
    const UnitInfo& baseInfo, int level, std::vector<UpgradeInfo> const& upgrades);

#endif // JAMTEMPLATE_UNIT_INFO_HPP
