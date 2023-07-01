#include "unit_info.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, UnitInfo const& data)
{
    j = nlohmann::json {
        // clang-format off
        { jk::unitType, data.type },
        {jk::description, data.description},

        { jk::experienceForLevelUp, data.experienceRequiredForLevelUp },
        { jk::experienceGainWhenKilled, data.experienceGainWhenKilled },

        { jk::hpInitial, data.hitpointsMax },

        { jk::damage, data.damage },
        { jk::armor, data.armor },

        { jk::attackTimerMax, data.attackTimerMax },
        { jk::movementSpeed, data.movementSpeed },

        { jk::colliderRadius, data.colliderRadius },

        { jk::cost, data.cost },
        { jk::unlockCost, data.unlockCost },


        { jk::animations, data.animations },
        { jk::ai, data.ai },

        { jk::upgrades, data.possibleUpgrades}
        // clang-format on
    };
}

void from_json(nlohmann::json const& j, UnitInfo& data)
{
    j.at(jk::unitType).get_to(data.type);
    j.at(jk::description).get_to(data.description);

    j.at(jk::experienceForLevelUp).get_to(data.experienceRequiredForLevelUp);
    j.at(jk::experienceGainWhenKilled).get_to(data.experienceGainWhenKilled);

    j.at(jk::hpInitial).get_to(data.hitpointsMax);

    j.at(jk::damage).get_to(data.damage);
    if (j.count(jk::armor) == 1) {
        j.at(jk::armor).get_to(data.armor);
    }

    j.at(jk::attackTimerMax).get_to(data.attackTimerMax);
    j.at(jk::movementSpeed).get_to(data.movementSpeed);

    j.at(jk::colliderRadius).get_to(data.colliderRadius);

    j.at(jk::cost).get_to(data.cost);
    j.at(jk::unlockCost).get_to(data.unlockCost);

    j.at(jk::ai).get_to(data.ai);
    j.at(jk::animations).get_to(data.animations);

    j.at(jk::upgrades).get_to(data.possibleUpgrades);
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

    info.armor.types.insert(info.armor.types.cbegin(), upg.armor.cbegin(), upg.armor.cend());
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
