#include "upgrade_value.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, const UpgradeValue& p)
{
    j = nlohmann::json { { jk::upgAdd, p.add }, { jk::upgFactor, p.factor } };
}

void from_json(const nlohmann::json& j, UpgradeValue& p)
{
    j.at(jk::upgAdd).get_to(p.add);
    j.at(jk::upgFactor).get_to(p.factor);
}
bool isDefault(UpgradeValue const& upg) { return upg.factor == 0.0f && upg.add == 0.0f; }
