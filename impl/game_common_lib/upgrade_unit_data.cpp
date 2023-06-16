#include "upgrade_unit_data.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, const UpgradeUnitData& p)
{
    j = nlohmann::json { { jk::unitType, p.unityType }, { jk::playerID, p.playerID },
        { jk::unitUpgrades, p.upgrade } };
}

void from_json(const nlohmann::json& j, UpgradeUnitData& p)
{
    j.at(jk::playerID).get_to(p.playerID);
    j.at(jk::unitType).get_to(p.unityType);
    j.at(jk::unitUpgrades).get_to(p.upgrade);
}
