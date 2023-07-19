#ifndef JAMTEMPLATE_BARRIER_INFO_HPP
#define JAMTEMPLATE_BARRIER_INFO_HPP

#include <nlohmann.hpp>
#include <vector2.hpp>

struct BarrierInfo {
    jt::Vector2f pos;
    float radius { 1.0f };
    float hpCurrent { 1.0f };
    float hpMax { 1.0f };
    int unitID { 0 };
    int playerID { 0 };
};

void to_json(nlohmann::json& j, const BarrierInfo& data);
void from_json(const nlohmann::json& j, BarrierInfo& data);

#endif // JAMTEMPLATE_BARRIER_INFO_HPP
