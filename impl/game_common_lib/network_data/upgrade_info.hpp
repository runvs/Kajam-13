#ifndef JAMTEMPLATE_UPGRADE_INFO_HPP
#define JAMTEMPLATE_UPGRADE_INFO_HPP

#include <network_data/upgrade_value.hpp>
#include <nlohmann.hpp>
#include <string>
#include <vector>

struct UpgradeInfo {
    std::string name;
    std::string iconPath;
    std::string description;
    int upgradeCost { 0 };

    UpgradeValue damage;
    UpgradeValue attackSpeed;
    UpgradeValue movementSpeed;
    UpgradeValue range;
    UpgradeValue hitpoints;
    UpgradeValue cost;
    UpgradeValue barrierRange;
    UpgradeValue barrierHp;
    std::vector<std::string> armor;
};

void to_json(nlohmann::json& j, const UpgradeInfo& p);
void from_json(const nlohmann::json& j, UpgradeInfo& p);

#endif // JAMTEMPLATE_UPGRADE_INFO_HPP
