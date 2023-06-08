
#include "arrow_info.hpp"
#include "json_keys.hpp"

void to_json(nlohmann::json& j, const ArrowInfo& data)
{
    j = nlohmann::json { { jk::positionX, data.currentPos.x },
        { jk::positionY, data.currentPos.y } };
}

void from_json(const nlohmann::json& j, ArrowInfo& data)
{
    j.at(jk::positionX).get_to(data.currentPos.x);
    j.at(jk::positionY).get_to(data.currentPos.y);
}
