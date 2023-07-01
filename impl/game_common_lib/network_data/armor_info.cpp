#include "armor_info.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, const ArmorInfo& data)
{
    j = nlohmann::json { { jk::armorType, data.types } };
}
void from_json(const nlohmann::json& j, ArmorInfo& data) { j.at(jk::armorType).get_to(data.types); }
