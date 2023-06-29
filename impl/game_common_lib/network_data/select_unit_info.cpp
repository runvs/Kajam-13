#include "select_unit_info.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, const SelectUnitInfo& p)
{
    j = nlohmann::json { { "unit1", p.unit1 }, { "unit2", p.unit2 },
        { "startingFunds", p.initialGold }, { "effect", p.effect } };
}

void from_json(const nlohmann::json& j, SelectUnitInfo& p)
{
    j.at("unit1").get_to(p.unit1);
    j.at("unit2").get_to(p.unit2);
    j.at("startingFunds").get_to(p.initialGold);
    j.at("effect").get_to(p.effect);
}

void to_json(nlohmann::json& j, const SelectUnitInfoCollection& p)
{
    j = nlohmann::json { { "startingUnits", p.m_selectUnitInfos } };
}
void from_json(const nlohmann::json& j, SelectUnitInfoCollection& p)
{
    j.at("startingUnits").get_to(p.m_selectUnitInfos);
}
