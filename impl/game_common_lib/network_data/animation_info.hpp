#ifndef JAMTEMPLATE_ANIMATION_INFO_HPP
#define JAMTEMPLATE_ANIMATION_INFO_HPP

#include <nlohmann.hpp>
#include <string>

struct AnimationInfo {
    std::string jsonfilename { "" };
};

void to_json(nlohmann::json& j, const AnimationInfo& p);
void from_json(const nlohmann::json& j, AnimationInfo& p);

#endif // JAMTEMPLATE_ANIMATION_INFO_HPP
