#ifndef JAMTEMPLATE_DAMAGE_INFO_HPP
#define JAMTEMPLATE_DAMAGE_INFO_HPP

#include <nlohmann.hpp>

struct DamageInfo {
    float damage;
    std::vector<std::string> damageTypes;
};

void to_json(nlohmann::json& j, const DamageInfo& data);
void from_json(const nlohmann::json& j, DamageInfo& data);

#endif // JAMTEMPLATE_DAMAGE_INFO_HPP
