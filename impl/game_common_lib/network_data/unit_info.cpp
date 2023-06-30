#include "unit_info.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, UnitInfo const& p)
{
    j = nlohmann::json {
        // clang-format off
        { jk::unitType, p.type },
        {jk::description, p.description},

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
    j.at(jk::description).get_to(p.description);

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
    applyUpgradeFloat(info.damage.damage, upg.damage);
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
    info.damage.damage = baseInfo.damage.damage * level;

    info.cost = baseInfo.cost * level;

    info.experienceGainWhenKilled = baseInfo.experienceGainWhenKilled * level;
    info.experienceRequiredForLevelUp
        = static_cast<int>(baseInfo.experienceRequiredForLevelUp * sqrt(level));

    for (auto const& upg : upgrades) {
        applyUpgrade(info, upg);
    }
    return info;
}