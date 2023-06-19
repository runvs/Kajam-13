#include "unit_server_to_client_data.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, const UnitServerToClientData& d)
{
    j = nlohmann::json {
        // clang-format off
        {jk::unitID, d.unitID},
        {jk::playerID, d.playerID},
        {jk::unitType, d.unitType},
        {jk::level, d.level},
        {jk::positionX, d.positionX},
        {jk::positionY, d.positionY},
        {jk::offsetX, d.offsetX},
        {jk::offsetY, d.offsetY},
        {jk::hpCurrent, d.hpCurrent},
        {jk::hpMax, d.hpMax},
        {jk::unitWalkingRight, d.unitWalkingRight}
        // clang-format on
    };
    if (d.unitAnim.has_value()) {
        j[jk::unitAnim] = d.unitAnim.value();
    }
}
void from_json(const nlohmann::json& j, UnitServerToClientData& d)
{
    j.at(jk::unitID).get_to(d.unitID);
    j.at(jk::playerID).get_to(d.playerID);
    j.at(jk::unitType).get_to(d.unitType);
    j.at(jk::level).get_to(d.level);
    j.at(jk::positionX).get_to(d.positionX);
    j.at(jk::positionY).get_to(d.positionY);
    j.at(jk::offsetX).get_to(d.offsetX);
    j.at(jk::offsetY).get_to(d.offsetY);
    j.at(jk::hpCurrent).get_to(d.hpCurrent);
    j.at(jk::hpMax).get_to(d.hpMax);
    j.at(jk::unitWalkingRight).get_to(d.unitWalkingRight);
    if (j.count(jk::unitAnim) == 1) {
        std::string anim;
        j.at(jk::unitAnim).get_to(anim);
        d.unitAnim = anim;
    }
}
