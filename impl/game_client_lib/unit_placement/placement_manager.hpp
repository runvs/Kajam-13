#ifndef JAMTEMPLATE_PLACEMENT_MANAGER_HPP
#define JAMTEMPLATE_PLACEMENT_MANAGER_HPP

#include "player_id_dispatcher.hpp"
#include <game_object.hpp>
#include <object_properties.hpp>
#include <shape.hpp>
#include <unit_placement/unit_id_manager.hpp>
#include <vector>

class PlacedUnit;

class PlacementManager : public jt::GameObject {
public:
    PlacementManager(int playerId, std::weak_ptr<PlayerIdDispatcher> playerIdDispatcher);
    std::vector<ObjectProperties> getPlacedUnits() const;
    void clearPlacedUnits();

    void setActive(bool active);

private:
    bool m_isActive { true };
    std::vector<std::shared_ptr<PlacedUnit>> m_placedUnits;
    UnitIdManager m_unitIdManager;
    std::weak_ptr<PlayerIdDispatcher> m_playerIdDispatcher;
    std::shared_ptr<jt::Shape> m_blockedUnitPlacementArea;
    int m_playerId;

    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;

    void placeUnit();
};

#endif // JAMTEMPLATE_PLACEMENT_MANAGER_HPP
