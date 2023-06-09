#ifndef JAMTEMPLATE_UPGRADE_UNIT_DATA_HPP
#define JAMTEMPLATE_UPGRADE_UNIT_DATA_HPP

#include <network_data/unit_info.hpp>
#include <nlohmann.hpp>
#include <string>

struct UpgradeUnitData {
    int playerID;
    std::string unityType;
    UpgradeInfo upgrade;
};

void to_json(nlohmann::json& j, const UpgradeUnitData& p);
void from_json(const nlohmann::json& j, UpgradeUnitData& p);

#endif // JAMTEMPLATE_UPGRADE_UNIT_DATA_HPP
