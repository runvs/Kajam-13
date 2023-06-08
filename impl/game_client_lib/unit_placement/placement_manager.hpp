#ifndef JAMTEMPLATE_PLACEMENT_MANAGER_HPP
#define JAMTEMPLATE_PLACEMENT_MANAGER_HPP

#include <game_object.hpp>
#include <game_object_collection.hpp>
#include <map/terrain.hpp>
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
    std::vector<ObjectProperties> getPlacedUnits() const;
    void clearPlacedUnits();

    void setActive(bool active);

private:
    std::shared_ptr<Terrain> m_world;
    std::shared_ptr<UnitInfoCollection> m_unitInfo;
    bool m_isActive { true };
    std::shared_ptr<jt::ObjectGroup<PlacedUnit>> m_placedUnits;
    jt::GameObjectCollection m_placedUnitsGO;
    UnitIdManager m_unitIdManager;
    std::weak_ptr<PlayerIdDispatcher> m_playerIdDispatcher;
    std::shared_ptr<jt::Shape> m_blockedUnitPlacementArea;
    int m_playerId;
    // TODO rework
    mutable std::string m_activeUnitType = "";

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;

    void placeUnit();
    bool fieldInUse(jt::Vector2f const& pos) const;
};

#endif // JAMTEMPLATE_PLACEMENT_MANAGER_HPP
