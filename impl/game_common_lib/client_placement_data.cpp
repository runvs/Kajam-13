
#include "client_placement_data.hpp"
#include <json_keys.hpp>
#include <nlohmann.hpp>

void to_json(nlohmann::json& j, const ClientPlacementData& data)
{
    j = nlohmann::json { { jk::units, data.m_units } };
}
void from_json(const nlohmann::json& j, ClientPlacementData& data)
{
    j.at(jk::units).get_to(data.m_units);
}
