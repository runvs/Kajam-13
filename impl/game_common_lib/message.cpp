
#include "message.h"

void to_json(nlohmann::json &j, const Message &message) {
    j = nlohmann::json{{"type", message.type},
                       {"data", message.data}};

}

void from_json(const nlohmann::json &j, Message &message) {
    j.at("type").get_to(message.type);
    j.at("data").get_to(message.data);
}
