#ifndef JAMTEMPLATE_SIMULATION_RESULT_DATA_HPP
#define JAMTEMPLATE_SIMULATION_RESULT_DATA_HPP

#include <network_data/arrow_info.hpp>
#include <network_data/explosion_info.hpp>
#include <network_data/shield_info.hpp>
#include <network_data/unit_server_to_client_data.hpp>
#include <nlohmann.hpp>
#include <map>
#include <vector>

struct SimulationResultDataForOneFrame {
    int m_frameId { 0 };
    std::vector<UnitServerToClientData> m_units;
    std::vector<ArrowInfo> m_arrows;
    std::map<int, int> m_playerHP {};
    std::vector<ExplosionInfo> m_explosions;
    std::vector<ShieldInfo> m_shields;
};

void to_json(nlohmann::json& j, const SimulationResultDataForOneFrame& data);
void from_json(const nlohmann::json& j, SimulationResultDataForOneFrame& data);

struct SimulationResultDataForAllFrames {
    std::vector<SimulationResultDataForOneFrame> allFrames;
};

void to_json(nlohmann::json& j, const SimulationResultDataForAllFrames& data);
void from_json(const nlohmann::json& j, SimulationResultDataForAllFrames& data);

#endif // JAMTEMPLATE_SIMULATION_RESULT_DATA_HPP
