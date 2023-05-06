
#include "client_end_placement_data.hpp"
#include "vector.hpp"
#include <nlohmann.hpp>

namespace jt {
void to_json(nlohmann::json& j, const jt::Vector2f& vec)
{
    j = nlohmann::json { { "x", vec.x }, { "y", vec.y } };
}

void from_json(const nlohmann::json& j, jt::Vector2f& vec)
{
    j.at("x").get_to(vec.x);
    j.at("y").get_to(vec.y);
}

} // namespace jt

void to_json(nlohmann::json& j, const ClientEndPlacementData& data)
{
    j = nlohmann::json { { "pos", data.m_position } };
}
void from_json(const nlohmann::json& j, ClientEndPlacementData& data)
{
    j.at("pos").get_to(data.m_position);
}
