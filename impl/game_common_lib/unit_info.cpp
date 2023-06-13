
#include "unit_info.hpp"
#include "json_keys.hpp"

void to_json(nlohmann::json& j, AnimationInfo const& p)
{
    j = nlohmann::json { { "jsonfilename", p.jsonfilename } };
}

void to_json(nlohmann::json& j, AiInfo const& p)
{
    j = nlohmann::json { { "type", p.type }, { "range", p.range }, { "arrowHeight", p.arrowHeight },
        { "arrowSpeed", p.arrowSpeed } };
}

void to_json(nlohmann::json& j, UnitInfo const& p)
{
    j = nlohmann::json {
        // clang-format off
        { "type", p.type },

        { jk::experienceForLevelUp, p.experienceForLevelUp },

        { "hitpoints", p.hitpoints },

        { "damage", p.damage },

        { jk::attackTimerMax, p.attackTimerMax },
        { "movementSpeed", p.movementSpeed },

        { "colliderRadius", p.colliderRadius },

        { jk::cost, p.cost },
        { jk::unlockCost, p.unlockCost },


        { "animations", p.animations },
        { "ai", p.ai }
        // clang-format on
    };
}

void from_json(nlohmann::json const& j, AnimationInfo& p)
{
    j.at("jsonfilename").get_to(p.jsonfilename);
}

void from_json(nlohmann::json const& j, AiInfo& p)
{
    j.at("type").get_to(p.type);
    j.at("range").get_to(p.range);
    if (j.count("arrowHeight") == 1) {
        j.at("arrowHeight").get_to(p.arrowHeight);
    }
    if (j.count("arrowSpeed") == 1) {
        j.at("arrowSpeed").get_to(p.arrowSpeed);
    }
}

void from_json(nlohmann::json const& j, UnitInfo& p)
{
    j.at("type").get_to(p.type);

    j.at(jk::experienceForLevelUp).get_to(p.experienceForLevelUp);
    p.experience = p.experienceForLevelUp;

    j.at("hitpoints").get_to(p.hitpoints);

    j.at("damage").get_to(p.damage);

    j.at(jk::attackTimerMax).get_to(p.attackTimerMax);
    j.at("movementSpeed").get_to(p.movementSpeed);

    j.at("colliderRadius").get_to(p.colliderRadius);

    j.at(jk::cost).get_to(p.cost);
    j.at(jk::unlockCost).get_to(p.unlockCost);

    j.at("ai").get_to(p.ai);
    j.at("animations").get_to(p.animations);
}
