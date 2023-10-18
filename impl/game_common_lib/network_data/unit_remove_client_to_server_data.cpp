#include "unit_remove_client_to_server_data.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, const UnitRemoveClientToServerData& p)
{
    j = nlohmann::json {
        // clang-format off
        {jk::unitID, p.unitID},
        {jk::playerID, p.playerID}
        // clang-format on
    };
}

void from_json(const nlohmann::json& j, UnitRemoveClientToServerData& p)
{
    j.at(jk::unitID).get_to(p.unitID);
    j.at(jk::playerID).get_to(p.playerID);
}
