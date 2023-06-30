#ifndef JAMTEMPLATE_UPGRADE_INFO_HPP
#define JAMTEMPLATE_UPGRADE_INFO_HPP

#include <network_data/upgrade_value.hpp>
#include <nlohmann.hpp>
#include <sprite.hpp>
#include <memory>
#include <string>

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

void to_json(nlohmann::json& j, const UpgradeInfo& p);
void from_json(const nlohmann::json& j, UpgradeInfo& p);

#endif // JAMTEMPLATE_UPGRADE_INFO_HPP
