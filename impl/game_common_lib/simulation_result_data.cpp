#include "simulation_result_data.hpp"
#include "json_keys.hpp"
void to_json(nlohmann::json& j, const SimulationResultDataForOneFrame& data)
{
    j = nlohmann::json { { jk::units, data.m_units }, { jk::arrows, data.m_arrows } };
}

void from_json(const nlohmann::json& j, SimulationResultDataForOneFrame& data)
{
    if (j.count(jk::units) == 1) {
        j.at(jk::units).get_to(data.m_units);
    }
    if (j.count(jk::arrows) == 1) {
        j.at(jk::arrows).get_to(data.m_arrows);
    }
}
