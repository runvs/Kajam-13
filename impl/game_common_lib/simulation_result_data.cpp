#include "simulation_result_data.hpp"
#include "json_keys.hpp"
void to_json(nlohmann::json& j, const SimulationResultData& data)
{
    j = nlohmann::json { { jk::units, data.m_unitPropertiesForOneFrame } };
}

void from_json(const nlohmann::json& j, SimulationResultData& data)
{
    if (j.count(jk::units) == 1) {
        j.at(jk::units).get_to(data.m_unitPropertiesForOneFrame);
    }
}
