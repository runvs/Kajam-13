
#include "client_end_placement_data.hpp"
#include "json_keys.hpp"
#include "vector.hpp"
#include <nlohmann.hpp>

namespace jt {
void to_json(nlohmann::json& j, const jt::Vector2f& pos, jt::Vector2f& offset)
{
    j = nlohmann::json { { jk::positionX, pos.x }, { jk::positionY, pos.y },
        { jk::offsetX, offset.x }, { jk::offsetY, offset.y } };
}

void from_json(const nlohmann::json& j, jt::Vector2f& pos, jt::Vector2f& offset)
{
    j.at(jk::positionX).get_to(pos.x);
    j.at(jk::positionY).get_to(pos.y);
    j.at(jk::offsetX).get_to(offset.x);
    j.at(jk::offsetY).get_to(offset.y);
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
