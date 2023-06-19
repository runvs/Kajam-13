#ifndef JAMTEMPLATE_PLACEMENT_MANAGER_HPP
#define JAMTEMPLATE_PLACEMENT_MANAGER_HPP

#include <game_object.hpp>
#include <game_object_collection.hpp>
#include <map/terrain.hpp>
#include <network_data/unit_client_to_server_data.hpp>
#include <object_group.hpp>
#include <object_properties.hpp>
#include <player_id_dispatcher.hpp>
#include <shape.hpp>
#include <unit_info_collection.hpp>
#include <unit_placement/unit_id_manager.hpp>
#include <vector>

class PlacedUnit;

class PlacementManager : public jt::GameObject {
public:
    PlacementManager(std::shared_ptr<Terrain> world, int playerId,
        std::weak_ptr<PlayerIdDispatcher> playerIdDispatcher,
        std::shared_ptr<UnitInfoCollection> unitInfo);
    std::vector<UnitClientToServerData> getPlacedUnits() const;
    std::shared_ptr<jt::ObjectGroup<PlacedUnit>> const& getPlacedUnitsGO() const;
    void clearPlacedUnits();

    void setActive(bool active);

    void addFunds(int funds);
    int getFunds() const;

    void unlockType(std::string const& type) const;

    void buyUpgrade(std::string const& unitType, const std::string& upgrade) const;
    std::vector<std::string> getPossibleUpgradesForUnit(std::string const& unitType) const;
    std::vector<std::string> getBoughtUpgradesForUnit(std::string const& unitType) const;

private:
    std::shared_ptr<Terrain> m_world;
    std::shared_ptr<UnitInfoCollection> m_unitInfo;
    bool m_isActive { true };
    std::array<bool, terrainWidthInChunks * terrainHeightInChunks> m_placedUnitsMap { false };
    std::shared_ptr<jt::ObjectGroup<PlacedUnit>> m_placedUnits;
    jt::GameObjectCollection m_placedUnitsGO;
    UnitIdManager m_unitIdManager;
    std::weak_ptr<PlayerIdDispatcher> m_playerIdDispatcher;
    std::shared_ptr<jt::Shape> m_blockedUnitPlacementAreas[3];
    int m_playerId;

    mutable std::string m_activeUnitType = "";

    mutable int m_availableFunds { 0 };

    mutable std::vector<std::string> m_unlockedTypes;
    mutable std::map<std::string, std::vector<std::string>> m_possibleUpgrades;
    mutable std::map<std::string, std::vector<std::string>> m_boughtUpgrades;

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;

    void placeUnit();
    bool& fieldInUse(int const x, int const y);
};

#endif // JAMTEMPLATE_PLACEMENT_MANAGER_HPP
