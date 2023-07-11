#include "ai_info.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, AiInfo const& aiInfo)
{
    j = nlohmann::json { { jk::aitype, aiInfo.type }, { jk::range, aiInfo.range },
        { jk::arrowHeight, aiInfo.arrowHeight }, { jk::arrowSpeed, aiInfo.arrowSpeed },
        { jk::shieldHp, aiInfo.shieldHp } };
}

void from_json(nlohmann::json const& j, AiInfo& aiInfo)
{
    j.at(jk::aitype).get_to(aiInfo.type);
    if (j.count(jk::range) == 1) {
        j.at(jk::range).get_to(aiInfo.range);
    }
    if (j.count(jk::arrowHeight) == 1) {
        j.at(jk::arrowHeight).get_to(aiInfo.arrowHeight);
    }
    if (j.count(jk::arrowSpeed) == 1) {
        j.at(jk::arrowSpeed).get_to(aiInfo.arrowSpeed);
    }
    if (j.count(jk::shieldHp) == 1) {
        j.at(jk::shieldHp).get_to(aiInfo.shieldHp);
    }
}
