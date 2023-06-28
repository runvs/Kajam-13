#ifndef JAMTEMPLATE_OBJECT_PROPERTIES_HPP
#define JAMTEMPLATE_OBJECT_PROPERTIES_HPP

#include <nlohmann.hpp>
#include <vector2.hpp>
#include <map>
#include <string>

struct ObjectProperties {
    std::map<std::string, bool> bools;
    std::map<std::string, float> floats;
    std::map<std::string, int> ints;
    std::map<std::string, std::string> strings;
};

void to_json(nlohmann::json& j, const ObjectProperties& data);

void from_json(const nlohmann::json& j, ObjectProperties& data);

#endif // JAMTEMPLATE_OBJECT_PROPERTIES_HPP
