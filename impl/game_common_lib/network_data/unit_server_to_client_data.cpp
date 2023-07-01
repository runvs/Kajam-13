#include "unit_server_to_client_data.hpp"
#include <json_keys.hpp>
#include <network_properties.hpp>

void to_json(nlohmann::json& j, const UnitServerToClientData& d)
{
    j = nlohmann::json {
        // clang-format off
        {jk::unitID, d.unitID},
        {jk::unitType, d.unitType},
        {jk::positionX, static_cast<int>(d.positionX * NetworkProperties::floatPrecision())},
        {jk::positionY, static_cast<int>(d.positionY * NetworkProperties::floatPrecision())},
        {jk::offsetX, static_cast<int>(d.offsetX * NetworkProperties::floatPrecision())},
        {jk::offsetY, static_cast<int>(d.offsetY * NetworkProperties::floatPrecision())},
        {jk::hpCurrent, static_cast<int>(d.hpCurrent * NetworkProperties::floatPrecision())},
        {jk::hpMax, static_cast<int>(d.hpMax * NetworkProperties::floatPrecision())}
        // clang-format on
    };
    if (d.unitAnim.has_value()) {
        j[jk::unitAnim] = d.unitAnim.value();
    }
    if (d.playerID == 1) {
        j[jk::playerID] = 1;
    }
    if (d.level != 1) {
        j[jk::level] = d.level;
    }
    if (d.experience != 0) {
        j[jk::experience] = d.experience;
    }
    if (d.unitWalkingRight) {
        j[jk::unitWalkingRight] = true;
    }
}
void from_json(const nlohmann::json& j, UnitServerToClientData& d)
{
    j.at(jk::unitID).get_to(d.unitID);
    d.playerID = j.value(jk::playerID, 0);
    j.at(jk::unitType).get_to(d.unitType);
    d.level = j.value(jk::level, 1);
    j.at(jk::positionX).get_to(d.positionX);
    d.positionX /= NetworkProperties::floatPrecision();
    j.at(jk::positionY).get_to(d.positionY);
    d.positionY /= NetworkProperties::floatPrecision();
    j.at(jk::offsetX).get_to(d.offsetX);
    d.offsetX /= NetworkProperties::floatPrecision();
    j.at(jk::offsetY).get_to(d.offsetY);
    d.offsetY /= NetworkProperties::floatPrecision();
    j.at(jk::hpCurrent).get_to(d.hpCurrent);
    d.hpCurrent /= NetworkProperties::floatPrecision();
    j.at(jk::hpMax).get_to(d.hpMax);
    d.hpMax /= NetworkProperties::floatPrecision();
    d.unitWalkingRight = j.value(jk::unitWalkingRight, false);
    if (j.count(jk::experience) == 1) {
        j.at(jk::experience).get_to(d.experience);
    }
    if (j.count(jk::unitAnim) == 1) {
        std::string anim;
        j.at(jk::unitAnim).get_to(anim);
        d.unitAnim = anim;
    }
}
