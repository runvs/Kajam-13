#ifndef JAMTEMPLATE_CLIENT_PLACEMENT_DATA_HPP
#define JAMTEMPLATE_CLIENT_PLACEMENT_DATA_HPP

#include "object_properties.hpp"
#include <nlohmann.hpp>
#include <vector.hpp>
#include <vector>

struct ClientPlacementData {
    std::vector<ObjectProperties> m_properties;
    std::vector<ObjectProperties> m_unitUpgrades;
};

void to_json(nlohmann::json& j, const ClientPlacementData& data);
void from_json(const nlohmann::json& j, ClientPlacementData& data);

#endif // JAMTEMPLATE_CLIENT_PLACEMENT_DATA_HPP
