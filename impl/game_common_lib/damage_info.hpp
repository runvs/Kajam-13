#ifndef JAMTEMPLATE_DAMAGE_INFO_HPP
#define JAMTEMPLATE_DAMAGE_INFO_HPP

#include <nlohmann.hpp>

struct DamageInfo {
    float damage;
};

void to_json(nlohmann::json& j, const DamageInfo& p);
void from_json(const nlohmann::json& j, DamageInfo& p);

#endif // JAMTEMPLATE_DAMAGE_INFO_HPP
