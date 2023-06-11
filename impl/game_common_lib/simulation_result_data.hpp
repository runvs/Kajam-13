#ifndef JAMTEMPLATE_SIMULATION_RESULT_DATA_HPP
#define JAMTEMPLATE_SIMULATION_RESULT_DATA_HPP

#include "arrow_info.hpp"
#include "object_properties.hpp"

struct SimulationResultDataForOneFrame {
    int m_frameId { 0 };
    std::vector<ObjectProperties> m_units;
    std::vector<ArrowInfo> m_arrows;
    std::map<int, int> m_playerHP {};
};

void to_json(nlohmann::json& j, const SimulationResultDataForOneFrame& data);
void from_json(const nlohmann::json& j, SimulationResultDataForOneFrame& data);

struct SimulationResultDataForAllFrames {
    std::vector<SimulationResultDataForOneFrame> allFrames;
};

#endif // JAMTEMPLATE_SIMULATION_RESULT_DATA_HPP
