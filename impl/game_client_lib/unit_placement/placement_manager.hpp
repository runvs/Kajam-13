#ifndef JAMTEMPLATE_PLACEMENT_MANAGER_HPP
#define JAMTEMPLATE_PLACEMENT_MANAGER_HPP

#include "audio/sound/sound_interface.hpp"
#include <game_object.hpp>
#include <game_object_collection.hpp>
#include <map/terrain.hpp>
#include <network_data/unit_client_to_server_data.hpp>
#include <network_data/unit_info.hpp>
#include <network_data/unit_info_collection.hpp>
#include <object_group.hpp>
#include <object_properties.hpp>
#include <player_id_dispatcher.hpp>
#include <shape.hpp>
#include <tween_collection.hpp>
#include <unit_placement/unit_id_manager.hpp>
#include <unit_placement/upgrade_manager.hpp>
#include <vector2.hpp>
#include <array>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

class PlacedUnit;

class PlacementManager : public jt::GameObject {
public:
    PlacementManager(std::shared_ptr<Terrain> world, int playerId,
        std::weak_ptr<PlayerIdDispatcher> playerIdDispatcher,
        std::shared_ptr<UnitInfoCollection> unitInfo);

    std::vector<UnitClientToServerData> getPlacedUnitDataForRoundStart() const;
    std::shared_ptr<jt::ObjectGroup<PlacedUnit>> const& getPlacedUnits() const;
    void clearPlacedUnits();

    void setActive(bool active);

    void addFunds(int funds);
    int getFunds() const;

    void unlockType(std::string const& type) const;

    void buyUnit(std::string const& type);

    std::shared_ptr<UpgradeManager> upgrades() const;

    void flashForUpgrade(std::string const& unitType);

private:
    std::shared_ptr<Terrain> m_world;
    std::shared_ptr<UnitInfoCollection> m_unitInfo;
    bool m_isActive { true };
    std::array<bool, terrainWidthInChunks * terrainHeightInChunks> m_placedUnitsMap { false };
    // Note: both ObjectGroup and GameObjectCollection is needed:
    // * ObjectGroup is not owning
    // * GaeObjectCollection only stores GameObjects
    std::shared_ptr<jt::ObjectGroup<PlacedUnit>> m_placedUnits;
    jt::GameObjectCollection m_placedUnitsGO;

    jt::TweenCollection m_tweens;

    UnitIdManager m_unitIdManager;
    std::weak_ptr<PlayerIdDispatcher> m_playerIdDispatcher;
    std::shared_ptr<jt::Shape> m_blockedUnitPlacementAreas[3];
    std::shared_ptr<jt::Shape> m_fieldHighlight { nullptr };
    std::map<std::string, std::shared_ptr<jt::Sprite>> m_imageUnits {};
    int m_playerId;

    mutable std::string m_activeUnitType = "";

    mutable int m_availableFunds { 0 };

    std::shared_ptr<jt::SoundInterface> m_sfxPlaceUnit { nullptr };
    std::shared_ptr<jt::SoundInterface> m_sfxBuyUpgrade { nullptr };

    mutable std::set<std::string> m_unlockedTypes {};

    mutable std::shared_ptr<UpgradeManager> m_upgrades { nullptr };

    mutable std::map<std::string, int> m_boughtUnits {};

    mutable int m_unitsUnlockedThisRound = 0;
    mutable int m_unitUnlocksAvailable = 1;

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;

    void placeUnit();

    bool isValidField(jt::Vector2f const& pos, int const x, int const y);
    bool& fieldInUse(int const x, int const y);
};

#endif // JAMTEMPLATE_PLACEMENT_MANAGER_HPP
