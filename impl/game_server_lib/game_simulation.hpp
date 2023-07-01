#ifndef JAMTEMPLATE_GAME_SIMULATION_HPP
#define JAMTEMPLATE_GAME_SIMULATION_HPP

#include <box2dwrapper/box2d_world_interface.hpp>
#include <close_combat_info.hpp>
#include <game_properties.hpp>
#include <log/logger_interface.hpp>
#include <map/terrain.hpp>
#include <network_data/arrow_info.hpp>
#include <network_data/unit_client_to_server_data.hpp>
#include <network_data/unit_info_collection.hpp>
#include <object_properties.hpp>
#include <player_info.hpp>
#include <server_network_connection.hpp>
#include <simulation_object_interface.hpp>
#include <simulation_result_sender_interface.hpp>
#include <unit_server_round_start_data.hpp>
#include <units/server_unit.hpp>
#include <upgrade_unit_data.hpp>
#include <vector2.hpp>
#include <world_info_interface.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <utility>

class GameSimulation : public WorldInfoInterface {
public:
    explicit GameSimulation(jt::LoggerInterface& logger, UnitInfoCollection& unitInfos);

    void rollNewMap();

    void prepareSimulationForNewRound();
    void addUnit(UnitClientToServerData const& unitData);
    void addUnitUpgrade(UpgradeUnitData const& upg);

    void performSimulation(SimulationResultSenderInterface& sender);

    // TODO think about using an optional here, as there might not be a valid target.
    // TODO otherwise, just return the passed in position?
    std::weak_ptr<SimulationObjectInterface> getClosestTargetTo(
        const jt::Vector2f& pos, int playerId) override;
    float getLocalSpeedFactorAt(jt::Vector2f const& pos, jt::Vector2f const& dir) override;
    jt::Vector2f getTerrainMappedFieldPosition(jt::Vector2f const& pos, int& x, int& y) override;
    float getTerrainMappedFieldHeight(jt::Vector2f const& pos) override;

    void spawnArrow(ArrowInfo const& arrowInfo, float delay) override;
    void scheduleAttack(CloseCombatInfo const& info, float delay) override;

    // clear all units, e.g. when resetting the server
    void clear();

    Terrain const& getTerrain() const { return *m_world; }

private:
    jt::LoggerInterface& m_logger;
    UnitInfoCollection& m_unitInfos;
    std::shared_ptr<jt::Box2DWorldInterface> m_b2World { nullptr };
    std::shared_ptr<Terrain> m_world;
    std::vector<UnitServerRoundStartData> m_unitInformationForRoundStart;
    std::vector<UpgradeUnitData> m_unitUpgrades;

    std::vector<std::shared_ptr<SimulationObjectInterface>> m_simulationObjects;

    std::vector<ArrowInfo> m_arrows;
    std::vector<std::pair<float, ArrowInfo>> m_arrowsToBeSpawned;
    std::vector<std::pair<float, CloseCombatInfo>> m_scheduledCloseCombatAttacks;
    std::map<int, int> m_playerHp { { 0, GP::InitialPlayerHP() }, { 1, GP::InitialPlayerHP() } };

    bool checkIfUnitIsUnique(UnitClientToServerData const& unitData);
    void handleScheduledAttacks(float timePerUpdate);
    void handleArrowsToBeSpawned(float timePerUpdate);
    void handleArrows(float timePerUpdate, SimulationResultDataForOneFrame& currentFrame);
    bool checkIsLastFrame(unsigned int i);
};

#endif // JAMTEMPLATE_GAME_SIMULATION_HPP
