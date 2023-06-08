#ifndef JAMTEMPLATE_CLIENT_END_PLACEMENT_DATA_HPP
#define JAMTEMPLATE_CLIENT_END_PLACEMENT_DATA_HPP

#include "object_properties.hpp"
#include <nlohmann.hpp>
#include <vector.hpp>
#include <vector>

struct ClientEndPlacementData {
    std::vector<ObjectProperties> m_properties;
};

void to_json(nlohmann::json& j, const ClientEndPlacementData& data);
void from_json(const nlohmann::json& j, ClientEndPlacementData& data);

#endif // JAMTEMPLATE_CLIENT_END_PLACEMENT_DATA_HPP
