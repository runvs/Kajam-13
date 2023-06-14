
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
        { jk::unitType, p.type },

        { jk::experienceForLevelUp, p.experienceRequiredForLevelUp },
        { jk::experienceGainWhenKilled, p.experienceGainWhenKilled },

        { jk::hpInitial, p.hitpointsMax },

        { jk::damage, p.damage },

        { jk::attackTimerMax, p.attackTimerMax },
        { jk::movementSpeed, p.movementSpeed },

        { jk::colliderRadius, p.colliderRadius },

        { jk::cost, p.cost },
        { jk::unlockCost, p.unlockCost },


        { jk::animations, p.animations },
        { jk::ai, p.ai }
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
    j.at(jk::unitType).get_to(p.type);

    j.at(jk::experienceForLevelUp).get_to(p.experienceRequiredForLevelUp);
    j.at(jk::experienceGainWhenKilled).get_to(p.experienceGainWhenKilled);

    j.at(jk::hpInitial).get_to(p.hitpointsMax);

    j.at(jk::damage).get_to(p.damage);

    j.at(jk::attackTimerMax).get_to(p.attackTimerMax);
    j.at(jk::movementSpeed).get_to(p.movementSpeed);

    j.at(jk::colliderRadius).get_to(p.colliderRadius);

    j.at(jk::cost).get_to(p.cost);
    j.at(jk::unlockCost).get_to(p.unlockCost);

    j.at(jk::ai).get_to(p.ai);
    j.at(jk::animations).get_to(p.animations);
}
