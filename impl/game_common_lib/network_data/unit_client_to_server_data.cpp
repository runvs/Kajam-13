#include "unit_client_to_server_data.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, const UnitClientToServerData& p)
{
    j = nlohmann::json {
        // clang-format off
        {jk::unitID, p.unitID},
        {jk::playerID, p.playerID},
        {jk::positionX, p.positionX},
        {jk::positionY, p.positionY},
        {jk::offsetX, p.offsetX},
        {jk::offsetY, p.offsetY},
        {jk::unitType, p.unitType}
        // clang-format on
    };
}
void from_json(const nlohmann::json& j, UnitClientToServerData& p)
{
    j.at(jk::unitID).get_to(p.unitID);
    j.at(jk::playerID).get_to(p.playerID);
    j.at(jk::positionX).get_to(p.positionX);
    j.at(jk::positionY).get_to(p.positionY);
    j.at(jk::offsetX).get_to(p.offsetX);
    j.at(jk::offsetY).get_to(p.offsetY);
    j.at(jk::unitType).get_to(p.unitType);
}
