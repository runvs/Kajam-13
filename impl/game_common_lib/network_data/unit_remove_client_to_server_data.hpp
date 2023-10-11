#ifndef JAMTEMPLATE_UNIT_REMOVE_CLIENT_TO_SERVER_DATA_HPP
#define JAMTEMPLATE_UNIT_REMOVE_CLIENT_TO_SERVER_DATA_HPP

#include <nlohmann.hpp>
#include <string>

struct UnitRemoveClientToServerData {
    int unitID { -1 };
    int playerID { -1 };
};

void to_json(nlohmann::json& j, const UnitRemoveClientToServerData& p);
void from_json(const nlohmann::json& j, UnitRemoveClientToServerData& p);

#endif // JAMTEMPLATE_UNIT_REMOVE_CLIENT_TO_SERVER_DATA_HPP
