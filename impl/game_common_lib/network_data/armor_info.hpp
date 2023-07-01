#ifndef JAMTEMPLATE_ARMOR_INFO_HPP
#define JAMTEMPLATE_ARMOR_INFO_HPP

#include <nlohmann.hpp>
#include <string>

struct ArmorInfo {
    std::vector<std::string> types {};
};

void to_json(nlohmann::json& j, const ArmorInfo& data);
void from_json(const nlohmann::json& j, ArmorInfo& data);

#endif // JAMTEMPLATE_ARMOR_INFO_HPP
