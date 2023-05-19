
#include "client_end_placement_data.hpp"
#include "json_keys.hpp"
#include "vector.hpp"
#include <nlohmann.hpp>

namespace jt {
void to_json(nlohmann::json& j, const jt::Vector2f& vec)
{
    j = nlohmann::json { { jk::positionX, vec.x }, { jk::positionY, vec.y } };
}

void from_json(const nlohmann::json& j, jt::Vector2f& vec)
{
    j.at(jk::positionX).get_to(vec.x);
    j.at(jk::positionY).get_to(vec.y);
}

} // namespace jt

void to_json(nlohmann::json& j, const ClientEndPlacementData& data)
{
    j = nlohmann::json { { jk::units, data.m_properties } };
}
void from_json(const nlohmann::json& j, ClientEndPlacementData& data)
{
    j.at(jk::units).get_to(data.m_properties);
}
