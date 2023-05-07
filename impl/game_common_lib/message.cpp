
#include "message.hpp"

void to_json(nlohmann::json& j, const Message& message)
{
    j = nlohmann::json { { "type", message.type }, { "playerId", message.playerId } };
    if (!message.data.empty()) {
        j["data"] = message.data;
    }
}

void from_json(const nlohmann::json& j, Message& message)
{
    j.at("type").get_to(message.type);
    if (j.count("data")) {
        j.at("data").get_to(message.data);
    }
    j.at("playerId").get_to(message.playerId);
}
