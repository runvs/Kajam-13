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

    std::shared_ptr<UnitInfoCollection> getUnitInfoCollection() const;
    std::vector<UnitClientToServerData> getPlacedUnitDataForRoundStart() const;
    std::shared_ptr<jt::ObjectGroup<PlacedUnit>> const& getPlacedUnits() const;
    void clearPlacedUnits();

    void setActive(bool active);

    void setRound(int round);

    bool canUnitBePlacedInField(jt::Vector2f const& pos, int const x, int const y);

    void addFunds(int funds);
    void addLoserBonus(int bonus);
    void spendFunds(int expenses);
    void recordFundsFromLastRound();
    int getFunds() const;
    void resetCreditDebt();
    std::string getActiveUnitType() const;
    void resetActiveUnitType() const;

    void unlockType(std::string const& type) const;

    void buyUnit(std::string const& type);
    void sellUnit(UnitInfo const& unitInfo) const;
    void sellUnitForPreviousRound(UnitInfo const& unitInfo) const;

    std::shared_ptr<UpgradeManager> upgrades() const;

    void flashForUpgrade(std::string const& unitType);

    bool& fieldInUse(int const x, int const y) const;

private:
    std::shared_ptr<Terrain> m_world;
    std::shared_ptr<UnitInfoCollection> m_unitInfo;
    bool m_isActive { true };
    mutable std::array<bool, terrainWidthInChunks * terrainHeightInChunks> m_placedUnitsMap {
        false
    };
    // Note: both ObjectGroup and GameObjectCollection is needed:
    // * ObjectGroup is not owning
    // * GaeObjectCollection only stores GameObjects
    std::shared_ptr<jt::ObjectGroup<PlacedUnit>> m_placedUnits;
    mutable jt::GameObjectCollection m_placedUnitsGO;

    jt::TweenCollection m_tweens;

    UnitIdManager m_unitIdManager;
    std::weak_ptr<PlayerIdDispatcher> m_playerIdDispatcher;
    std::shared_ptr<jt::Shape> m_blockedUnitPlacementAreas[3];
    std::shared_ptr<jt::Shape> m_fieldHighlight { nullptr };
    std::map<std::string, std::shared_ptr<jt::Sprite>> m_imageUnits {};
    int m_playerId;

    mutable std::string m_activeUnitType = "";

    int m_round { 0 };
    mutable int m_availableFunds { 0 };
    mutable int m_creditDebt { 0 };
    int m_creditDebtFromLastRound { 0 };
    mutable int m_fundsFromLastRound { 0 };
    mutable int m_fundsGainedInLastRound { 0 };
    mutable int m_looserBonusFromLastRound { 0 };

    std::shared_ptr<jt::SoundInterface> m_sfxPlaceUnit { nullptr };
    std::shared_ptr<jt::SoundInterface> m_sfxBuyUpgrade { nullptr };

    mutable std::set<std::string> m_unlockedTypes {};

    mutable std::shared_ptr<UpgradeManager> m_upgrades { nullptr };

    mutable std::map<std::string, int> m_boughtUnits {};

    mutable int m_unitsUnlockedThisRound = 0;
    mutable int m_unitUnlocksAvailable = 1;
    mutable bool m_showUnlockUnitWindow { false };

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;

    void placeUnit();

    void drawGoldStatistics() const;
};

#endif // JAMTEMPLATE_PLACEMENT_MANAGER_HPP
