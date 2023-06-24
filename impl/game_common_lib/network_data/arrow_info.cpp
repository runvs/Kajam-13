
#include "arrow_info.hpp"
#include <json_keys.hpp>
#include <network_properties.hpp>

void to_json(nlohmann::json& j, const ArrowInfo& data)
{
    j = nlohmann::json {
        // clang-format off
        { jk::positionX, static_cast<int>(data.currentPos.x * NetworkProperties::floatPrecision()) },
        { jk::positionY, static_cast<int>(data.currentPos.y * NetworkProperties::floatPrecision()) }
        // clang-format on
    };

    if (data.arrowScale != 1.0f) {
        j[jk::scale] = static_cast<int>(data.arrowScale * NetworkProperties::floatPrecision());
    }
}

void from_json(const nlohmann::json& j, ArrowInfo& data)
{
    j.at(jk::positionX).get_to(data.currentPos.x);
    data.currentPos.x /= NetworkProperties::floatPrecision();
    j.at(jk::positionY).get_to(data.currentPos.y);
    data.currentPos.y /= NetworkProperties::floatPrecision();

    if (j.count(jk::scale)) {
        j.at(jk::scale).get_to(data.arrowScale);
        data.arrowScale /= NetworkProperties::floatPrecision();
    } else {
        data.arrowScale = 1.0f;
    }
}
