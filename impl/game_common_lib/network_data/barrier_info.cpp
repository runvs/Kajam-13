#include "barrier_info.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, const BarrierInfo& data)
{
    j = nlohmann::json {

        // clang-format off
        { jk::positionX, data.pos.x },
        { jk::positionY, data.pos.y },
        { jk::range, data.radius },
        { jk::hpCurrent, data.hpCurrent },
        { jk::hpMax, data.hpMax },
        { jk::unitID, data.unitID },
        { jk::playerID, data.playerID }

        // clang-format on
    };
}

void from_json(const nlohmann::json& j, BarrierInfo& data)
{
    j.at(jk::positionX).get_to(data.pos.x);
    j.at(jk::positionY).get_to(data.pos.y);
    j.at(jk::range).get_to(data.radius);
    j.at(jk::hpCurrent).get_to(data.hpCurrent);
    j.at(jk::hpMax).get_to(data.hpMax);
    j.at(jk::unitID).get_to(data.unitID);
    j.at(jk::playerID).get_to(data.playerID);
}
