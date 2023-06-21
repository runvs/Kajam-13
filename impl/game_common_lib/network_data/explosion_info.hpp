#ifndef JAMTEMPLATE_EXPLOSION_INFO_HPP
#define JAMTEMPLATE_EXPLOSION_INFO_HPP
#include "vector.hpp"
#include <nlohmann.hpp>

struct ExplosionInfo {
    jt::Vector2f position { 0.0f, 0.0f };
    float radius { 1.0f };
};

void to_json(nlohmann::json& j, const ExplosionInfo& data);
void from_json(const nlohmann::json& j, ExplosionInfo& data);

#endif // JAMTEMPLATE_EXPLOSION_INFO_HPP
