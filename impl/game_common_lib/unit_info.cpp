#include "unit_info.hpp"
#include "json_keys.hpp"
#include <nlohmann.hpp>

void to_json(nlohmann::json& j, AiInfo const& p)
{
    j = nlohmann::json { { "type", p.type }, { jk::range, p.range },
        { "arrowHeight", p.arrowHeight }, { "arrowSpeed", p.arrowSpeed } };
}

void from_json(nlohmann::json const& j, AiInfo& p)
{
    j.at("type").get_to(p.type);
    j.at(jk::range).get_to(p.range);
    if (j.count("arrowHeight") == 1) {
        j.at("arrowHeight").get_to(p.arrowHeight);
    }
    if (j.count("arrowSpeed") == 1) {
        j.at("arrowSpeed").get_to(p.arrowSpeed);
    }
}

void to_json(nlohmann::json& j, AnimationInfo const& p)
{
    j = nlohmann::json { { "jsonfilename", p.jsonfilename } };
}

void from_json(nlohmann::json const& j, AnimationInfo& p)
{
    j.at("jsonfilename").get_to(p.jsonfilename);
}

void to_json(nlohmann::json& j, const UpgradeInfo& p)
{
    j = nlohmann::json {
        // clang-format off
        { jk::upgName, p.name },

        { jk::damage, p.damage },
        { jk::attackTimerMax, p.attackSpeed },
        { jk::movementSpeed, p.movementSpeed },
        { jk::range, p.range },
        { jk::hpInitial, p.hitpoints },
        { jk::cost, p.cost },
        { jk::upgCost, p.upgradeCost }
        // clang-format on
    };
}
void from_json(const nlohmann::json& j, UpgradeInfo& p)
{
    j.at(jk::upgName).get_to(p.name);
    j.at(jk::upgCost).get_to(p.upgradeCost);

    if (j.count(jk::damage) == 1) {
        j.at(jk::damage).get_to(p.damage);
    }
    if (j.count(jk::attackTimerMax) == 1) {
        j.at(jk::attackTimerMax).get_to(p.attackSpeed);
    }
    if (j.count(jk::movementSpeed) == 1) {
        j.at(jk::movementSpeed).get_to(p.movementSpeed);
    }
    if (j.count(jk::range) == 1) {
        j.at(jk::range).get_to(p.range);
    }
    if (j.count(jk::hpInitial) == 1) {
        j.at(jk::hpInitial).get_to(p.hitpoints);
    }
    if (j.count(jk::cost) == 1) {
        j.at(jk::cost).get_to(p.cost);
    }
}

void to_json(nlohmann::json& j, const UpgradeValue& p)
{
    j = nlohmann::json { { jk::upgAdd, p.add }, { jk::upgFactor, p.factor } };
}
void from_json(const nlohmann::json& j, UpgradeValue& p)
{
    j.at(jk::upgAdd).get_to(p.add);
    j.at(jk::upgFactor).get_to(p.factor);
}

void to_json(nlohmann::json& j, UnitInfo const& p)
{
    j = nlohmann::json {
        // clang-format off
        { jk::unitType, p.type },

        { jk::experienceForLevelUp, p.experienceRequiredForLevelUp },
        { jk::experienceGainWhenKilled, p.experienceGainWhenKilled },

        { jk::hpInitial, p.hitpointsMax },

        { jk::damage, p.damage },

        { jk::attackTimerMax, p.attackTimerMax },
        { jk::movementSpeed, p.movementSpeed },

        { jk::colliderRadius, p.colliderRadius },

        { jk::cost, p.cost },
        { jk::unlockCost, p.unlockCost },


        { jk::animations, p.animations },
        { jk::ai, p.ai },

        { jk::upgrades, p.possibleUpgrades}
        // clang-format on
    };
}

void from_json(nlohmann::json const& j, UnitInfo& p)
{
    j.at(jk::unitType).get_to(p.type);

    j.at(jk::experienceForLevelUp).get_to(p.experienceRequiredForLevelUp);
    j.at(jk::experienceGainWhenKilled).get_to(p.experienceGainWhenKilled);

    j.at(jk::hpInitial).get_to(p.hitpointsMax);

    j.at(jk::damage).get_to(p.damage);

    j.at(jk::attackTimerMax).get_to(p.attackTimerMax);
    j.at(jk::movementSpeed).get_to(p.movementSpeed);

    j.at(jk::colliderRadius).get_to(p.colliderRadius);

    j.at(jk::cost).get_to(p.cost);
    j.at(jk::unlockCost).get_to(p.unlockCost);

    j.at(jk::ai).get_to(p.ai);
    j.at(jk::animations).get_to(p.animations);

    j.at(jk::upgrades).get_to(p.possibleUpgrades);
}

namespace {
void applyUpgradeInt(int& value, const UpgradeValue& upg)
{
    value = static_cast<int>(value * (1.0f + upg.factor) + upg.add);
}
void applyUpgradeFloat(float& value, const UpgradeValue& upg)
{
    value = value * (1.0f + upg.factor) + upg.add;
}

} // namespace

void applyUpgrade(UnitInfo& info, const UpgradeInfo& upg)
{
    applyUpgradeFloat(info.hitpointsMax, upg.hitpoints);
    applyUpgradeFloat(info.damage, upg.damage);
    applyUpgradeInt(info.cost, upg.cost);
    applyUpgradeFloat(info.attackTimerMax, upg.attackSpeed);
    applyUpgradeFloat(info.movementSpeed, upg.movementSpeed);
    applyUpgradeFloat(info.ai.range, upg.range);
}

UnitInfo getUnitInfoWithLevelAndUpgrades(
    const UnitInfo& baseInfo, int level, std::vector<UpgradeInfo> const& upgrades)
{
    UnitInfo info = baseInfo;

    info.hitpointsMax = baseInfo.hitpointsMax * level;
    info.damage = baseInfo.damage * level;

    info.cost = baseInfo.cost * level;

    info.experienceGainWhenKilled = baseInfo.experienceGainWhenKilled * level;
    info.experienceRequiredForLevelUp
        = static_cast<int>(baseInfo.experienceRequiredForLevelUp * sqrt(level));

    for (auto const& upg : upgrades) {
        applyUpgrade(info, upg);
    }
    return info;
}
