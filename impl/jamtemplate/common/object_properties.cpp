#include "object_properties.hpp"

void to_json(nlohmann::json& j, ObjectProperties const& data)
{
    j = nlohmann::json { { "floats", data.floats }, { "ints", data.ints } };
    //    if (!data.bools.empty()) {
    //        j["bools"] = data.bools;
    //    }
    //    if (!data.floats.empty()) {
    //        j["floats"] = data.floats;
    //    }
    //    if (!data.floats.empty()) {
    //        j["ints"] = data.ints;
    //    }
    //    if (!data.floats.empty()) {
    //        j["strings"] = data.strings;
    //    }
}

void from_json(nlohmann::json const& j, ObjectProperties& data)
{
    std::string str = j.dump(-1);
    if (j.count("bools") == 1) {
        j.at("bools").get_to(data.bools);
    }
    if (j.count("floats") == 1) {
        j.at("floats").get_to(data.floats);
    }
    if (j.count("ints") == 1) {
        j.at("ints").get_to(data.ints);
    }
    if (j.count("strings") == 1) {
        j.at("strings").get_to(data.strings);
    }
}
