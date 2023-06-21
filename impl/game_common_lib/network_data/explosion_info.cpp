#include "explosion_info.hpp"
#include <json_keys.hpp>
#include <network_properties.hpp>

void to_json(nlohmann::json& j, const ExplosionInfo& data)
{
    j = nlohmann::json {
        // clang-format off
        {jk::positionX, static_cast<int>(data.position.x * NetworkProperties::floatPrecision()) },
        {jk::positionY, static_cast<int>(data.position.y * NetworkProperties::floatPrecision()) },
        {jk::scale,     static_cast<int>(data.radius * NetworkProperties::floatPrecision()) }
        // clang-format on
    };
}

void from_json(const nlohmann::json& j, ExplosionInfo& data)
{
    j.at(jk::positionX).get_to(data.position.x);
    data.position.x /= NetworkProperties::floatPrecision();
    j.at(jk::positionY).get_to(data.position.y);
    data.position.y /= NetworkProperties::floatPrecision();
    j.at(jk::scale).get_to(data.radius);
    data.radius /= NetworkProperties::floatPrecision();
}
