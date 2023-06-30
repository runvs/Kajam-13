#include "animation_info.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, AnimationInfo const& p)
{
    j = nlohmann::json { { jk::filename, p.jsonfilename } };
}

void from_json(nlohmann::json const& j, AnimationInfo& p)
{
    j.at(jk::filename).get_to(p.jsonfilename);
}
