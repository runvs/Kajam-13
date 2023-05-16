
#include "message.hpp"

void to_json(nlohmann::json& j, const Message& message)
{
    j = nlohmann::json { { "t", message.type }, { "pid", message.playerId } };
    if (!message.data.empty()) {
        j["d"] = message.data;
    }
}

void from_json(const nlohmann::json& j, Message& message)
{
    j.at("t").get_to(message.type);
    if (j.count("d")) {
        j.at("d").get_to(message.data);
    }
    j.at("pid").get_to(message.playerId);
}
