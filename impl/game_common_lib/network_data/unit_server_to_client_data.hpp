#ifndef JAMTEMPLATE_UNIT_SERVER_TO_CLIENT_DATA_HPP
#define JAMTEMPLATE_UNIT_SERVER_TO_CLIENT_DATA_HPP

#include <nlohmann.hpp>
#include <optional>
#include <string>

struct UnitServerToClientData {
    int unitID { 0 };
    int playerID { 0 };
    std::string unitType { "" };
    int level { 1 };

    float positionX { 0.0f };
    float positionY { 0.0f };
    float offsetX { 0.0f };
    float offsetY { 0.0f };

    float hpCurrent { 1.0f };
    float hpMax { 1.0f };

    bool unitWalkingRight { false };
    // TODO think about adding upgrades here, but they should actually be stored on the client

    // TODO think about adding exp

    std::optional<std::string> unitAnim {};
};

void to_json(nlohmann::json& j, const UnitServerToClientData& d);
void from_json(const nlohmann::json& j, UnitServerToClientData& d);

#endif // JAMTEMPLATE_UNIT_SERVER_TO_CLIENT_DATA_HPP
