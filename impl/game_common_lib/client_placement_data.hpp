#ifndef JAMTEMPLATE_CLIENT_PLACEMENT_DATA_HPP
#define JAMTEMPLATE_CLIENT_PLACEMENT_DATA_HPP

#include <network_data/unit_client_to_server_data.hpp>
#include <nlohmann.hpp>
#include <object_properties.hpp>
#include <vector>

struct ClientPlacementData {
    std::vector<UnitClientToServerData> m_units;
};

void to_json(nlohmann::json& j, const ClientPlacementData& data);
void from_json(const nlohmann::json& j, ClientPlacementData& data);

#endif // JAMTEMPLATE_CLIENT_PLACEMENT_DATA_HPP
