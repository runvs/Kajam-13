#ifndef JAMTEMPLATE_SIMULATION_RESULT_DATA_HPP
#define JAMTEMPLATE_SIMULATION_RESULT_DATA_HPP

#include "object_properties.hpp"
struct SimulationResultData {
    std::vector<ObjectProperties> m_unitPropertiesForOneFrame;
};

void to_json(nlohmann::json& j, const SimulationResultData& data);

void from_json(const nlohmann::json& j, SimulationResultData& data);

#endif // JAMTEMPLATE_SIMULATION_RESULT_DATA_HPP
