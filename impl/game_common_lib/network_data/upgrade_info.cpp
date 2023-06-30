#include "upgrade_info.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, const UpgradeInfo& p)
{
    j = nlohmann::json {
        // clang-format off
        { jk::upgName, p.name },
        { jk::upgIconPath, p.iconPath },
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
    j.at(jk::upgIconPath).get_to(p.iconPath);
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
