#ifndef JAMTEMPLATE_CLIENT_END_PLACEMENT_DATA_HPP
#define JAMTEMPLATE_CLIENT_END_PLACEMENT_DATA_HPP

#include <nlohmann.hpp>
#include <vector.hpp>

struct ClientEndPlacementData {
    // TODO replace by vector of positions or whatever is needed
    jt::Vector2f m_position;
};

void to_json(nlohmann::json& j, const ClientEndPlacementData& data);

void from_json(const nlohmann::json& j, ClientEndPlacementData& data);

#endif // JAMTEMPLATE_CLIENT_END_PLACEMENT_DATA_HPP
