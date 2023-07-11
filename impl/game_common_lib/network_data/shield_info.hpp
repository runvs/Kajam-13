#ifndef JAMTEMPLATE_SHIELD_INFO_HPP
#define JAMTEMPLATE_SHIELD_INFO_HPP

#include <nlohmann.hpp>
#include <vector2.hpp>

struct ShieldInfo {
    jt::Vector2f pos;
    float radius { 1.0f };
    float hpCurrent { 1.0f };
    float hpMax { 1.0f };
    int unitID { 0 };
    int playerID { 0 };
};

void to_json(nlohmann::json& j, const ShieldInfo& data);
void from_json(const nlohmann::json& j, ShieldInfo& data);

#endif // JAMTEMPLATE_SHIELD_INFO_HPP
