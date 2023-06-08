
#include "unit_info.hpp"
#include "json_keys.hpp"

void to_json(nlohmann::json& j, AnimationInfo const& p)
{
    j = nlohmann::json { { "jsonfilename", p.jsonfilename } };
}

void to_json(nlohmann::json& j, AiInfo const& p)
{
    j = nlohmann::json { { "type", p.type }, { "range", p.range } };
}

void to_json(nlohmann::json& j, UnitInfo const& p)
{
    j = nlohmann::json { { "hitpoints", p.hitpoints }, { "experience", p.experience },
        { "movementSpeed", p.movementSpeed }, { "colliderRadius", p.colliderRadius },
        { "damage", p.damage }, { jk::attackTimerMax, p.attackTimerMax }, { "ai", p.ai },
        { "animations", p.animations }, { "type", p.type } };
}

void from_json(nlohmann::json const& j, AnimationInfo& p)
{
    j.at("jsonfilename").get_to(p.jsonfilename);
}

void from_json(nlohmann::json const& j, AiInfo& p)
{
    j.at("type").get_to(p.type);
    j.at("range").get_to(p.range);
}

void from_json(nlohmann::json const& j, UnitInfo& p)
{
    j.at("hitpoints").get_to(p.hitpoints);
    j.at("experience").get_to(p.experience);
    j.at("movementSpeed").get_to(p.movementSpeed);
    j.at("colliderRadius").get_to(p.colliderRadius);
    j.at("damage").get_to(p.damage);
    j.at(jk::attackTimerMax).get_to(p.attackTimerMax);
    j.at("ai").get_to(p.ai);
    j.at("animations").get_to(p.animations);
    j.at("type").get_to(p.type);
}
