
#include "arrow_info.hpp"
#include "json_keys.hpp"
#include <network_properties.hpp>

void to_json(nlohmann::json& j, const ArrowInfo& data)
{
    j = nlohmann::json {
        { jk::positionX,
            static_cast<int>(data.currentPos.x * NetworkProperties::floatPrecision()) },
        { jk::positionY, static_cast<int>(data.currentPos.y * NetworkProperties::floatPrecision()) }
    };
}

void from_json(const nlohmann::json& j, ArrowInfo& data)
{
    j.at(jk::positionX).get_to(data.currentPos.x);
    data.currentPos.x /= NetworkProperties::floatPrecision();
    j.at(jk::positionY).get_to(data.currentPos.y);
    data.currentPos.y /= NetworkProperties::floatPrecision();
}
