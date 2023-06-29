#ifndef JAMTEMPLATE_SELECT_UNIT_INFO_HPP
#define JAMTEMPLATE_SELECT_UNIT_INFO_HPP

#include <nlohmann.hpp>
#include <string>
#include <vector>

struct SelectUnitInfo {
    std::string unit1 { "" };
    std::string unit2 { "" };
    int initialGold { 0 };
    std::string effect { "No Benefit" };
};

void to_json(nlohmann::json& j, const SelectUnitInfo& p);
void from_json(const nlohmann::json& j, SelectUnitInfo& p);

struct SelectUnitInfoCollection {
    std::vector<SelectUnitInfo> m_selectUnitInfos;
};

void to_json(nlohmann::json& j, const SelectUnitInfoCollection& p);
void from_json(const nlohmann::json& j, SelectUnitInfoCollection& p);

#endif // JAMTEMPLATE_SELECT_UNIT_INFO_HPP
