
#include "message.hpp"
#include <json_keys.hpp>

void to_json(nlohmann::json& j, const Message& message)
{
    j = nlohmann::json { { jk::messageType, message.type }, { jk::playerID, message.playerId } };
    if (!message.data.empty()) {
        j[jk::messageData] = message.data;
    }
}

void from_json(const nlohmann::json& j, Message& message)
{
    j.at(jk::messageType).get_to(message.type);
    if (j.count(jk::messageData)) {
        j.at(jk::messageData).get_to(message.data);
    }
    j.at(jk::playerID).get_to(message.playerId);
}
