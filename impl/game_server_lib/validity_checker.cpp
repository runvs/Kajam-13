#include "validity_checker.hpp"
#include "map/terrain.hpp"

bool ValidityChecker::checkIfUnitIDsAreUnique(
    std::vector<UnitServerRoundStartData> const& existingUnits,
    UnitClientToServerData const& newUnitData)
{
    for (auto const& unitData : existingUnits) {
        if (unitData.unitClientToServerData.unitID == newUnitData.unitID
            && unitData.unitClientToServerData.playerID == newUnitData.playerID) {

            return false;
        }
    }
    return true;
}
bool ValidityChecker::checkIfUnitPlacementIsValid(UnitClientToServerData const& newUnitData)
{
    if (newUnitData.positionX < 0) {
        return false;
    }
    if (newUnitData.positionX >= terrainWidthInChunks * terrainChunkSizeInPixel) {
        return false;
    }

    if (newUnitData.positionY < 0) {
        return false;
    } else if (newUnitData.positionY >= terrainHeightInChunks * terrainChunkSizeInPixel) {
        return false;
    }
    return true;
}
