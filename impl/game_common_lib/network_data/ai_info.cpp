#include "ai_info.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, AiInfo const& p)
{
    j = nlohmann::json { { jk::aitype, p.type }, { jk::range, p.range },
        { jk::arrowHeight, p.arrowHeight }, { jk::arrowSpeed, p.arrowSpeed } };
}

void from_json(nlohmann::json const& j, AiInfo& p)
{
    j.at(jk::aitype).get_to(p.type);
    j.at(jk::range).get_to(p.range);
    if (j.count(jk::arrowHeight) == 1) {
        j.at(jk::arrowHeight).get_to(p.arrowHeight);
    }
    if (j.count(jk::arrowSpeed) == 1) {
        j.at(jk::arrowSpeed).get_to(p.arrowSpeed);
    }
}
