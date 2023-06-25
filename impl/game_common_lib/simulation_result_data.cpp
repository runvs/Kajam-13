#include "simulation_result_data.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, const SimulationResultDataForOneFrame& data)
{
    j = nlohmann::json {
        // clang-format off
        { jk::units, data.m_units },
        { jk::frameID, data.m_frameId },
        { jk::playerHp, data.m_playerHP }
        // clang-format on
    };
    if (!data.m_arrows.empty()) {
        j[jk::arrows] = data.m_arrows;
    }
    if (!data.m_explosions.empty()) {
        j[jk::explosions] = data.m_explosions;
    }
}

void from_json(const nlohmann::json& j, SimulationResultDataForOneFrame& data)
{
    if (j.count(jk::units) == 1) {
        j.at(jk::units).get_to(data.m_units);
    }
    if (j.count(jk::arrows) == 1) {
        j.at(jk::arrows).get_to(data.m_arrows);
    }
    if (j.count(jk::playerHp) == 1) {
        j.at(jk::playerHp).get_to(data.m_playerHP);
    }
    if (j.count(jk::explosions)) {
        j.at(jk::explosions).get_to(data.m_explosions);
    }
}

void to_json(nlohmann::json& j, const SimulationResultDataForAllFrames& data)
{
    j = nlohmann::json { { jk::allFrames, data.allFrames } };
}

void from_json(const nlohmann::json& j, SimulationResultDataForAllFrames& data)
{
    j.at(jk::allFrames).get_to(data.allFrames);
}
