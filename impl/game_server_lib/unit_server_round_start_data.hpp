#ifndef JAMTEMPLATE_UNIT_SERVER_ROUND_START_DATA_HPP
#define JAMTEMPLATE_UNIT_SERVER_ROUND_START_DATA_HPP

#include <network_data/unit_client_to_server_data.hpp>

struct UnitServerRoundStartData {
    UnitClientToServerData unitClientToServerData;
    int level { 1 };
    int experience { 0 };
};

#endif // JAMTEMPLATE_UNIT_SERVER_ROUND_START_DATA_HPP
