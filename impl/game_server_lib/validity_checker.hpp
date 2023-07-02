#ifndef JAMTEMPLATE_VALIDITY_CHECKER_HPP
#define JAMTEMPLATE_VALIDITY_CHECKER_HPP

#include <network_data/unit_client_to_server_data.hpp>
#include <unit_server_round_start_data.hpp>
#include <vector>

namespace ValidityChecker {

bool checkIfUnitIDsAreUnique(std::vector<UnitServerRoundStartData> const& existingUnits,
    UnitClientToServerData const& newUnitData);

bool checkIfUnitPlacementIsValid(UnitClientToServerData const& newUnitData);

}; // namespace ValidityChecker

#endif // JAMTEMPLATE_VALIDITY_CHECKER_HPP
