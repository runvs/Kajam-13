#ifndef JAMTEMPLATE_UNIT_CLIENT_TO_SERVER_DATA_HPP
#define JAMTEMPLATE_UNIT_CLIENT_TO_SERVER_DATA_HPP

#include <nlohmann.hpp>
#include <string>

struct UnitClientToServerData {
    int unitID { -1 };
    int playerID { -1 };
    float positionX { 0.0f };
    float positionY { 0.0f };
    float offsetX { 0.0f };
    float offsetY { 0.0f };
    std::string unitType { "" };
};

void to_json(nlohmann::json& j, const UnitClientToServerData& p);
void from_json(const nlohmann::json& j, UnitClientToServerData& p);

#endif // JAMTEMPLATE_UNIT_CLIENT_TO_SERVER_DATA_HPP
