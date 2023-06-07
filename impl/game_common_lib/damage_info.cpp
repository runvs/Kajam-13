#include "damage_info.hpp"
#include "json_keys.hpp"

void to_json(nlohmann::json& j, const DamageInfo& p)
{
    j = nlohmann::json { jk::damage, p.damage };
}

void from_json(const nlohmann::json& j, DamageInfo& p) { j.at(jk::damage).get_to(p.damage); }
