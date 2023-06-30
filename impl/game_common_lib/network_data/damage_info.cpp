#include "damage_info.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, const DamageInfo& data)
{
    j = nlohmann::json { { jk::damage, data.damage } };
    if (!data.damageTypes.empty()) {
        j[jk::damageTypes] = data.damageTypes;
    }
}

void from_json(const nlohmann::json& j, DamageInfo& data)
{
    j.at(jk::damage).get_to(data.damage);
    if (j.count(jk::damageTypes) == 1) {
        j.at(jk::damageTypes).get_to(data.damageTypes);
    }
}
