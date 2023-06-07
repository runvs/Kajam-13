
#ifndef JAMTEMPLATE_GAME_SIMULATION_HPP
#define JAMTEMPLATE_GAME_SIMULATION_HPP

#include <log/logger_interface.hpp>
#include <map/terrain.hpp>
#include <object_properties.hpp>
#include <player_info.hpp>
#include <server_network_connection.hpp>
#include <server_unit.hpp>
#include <simulation_object_interface.hpp>
#include <simulation_result_message_sender.hpp>
#include <unit_info_collection.hpp>
#include <world_info_interface.hpp>
#include <map>
#include <memory>
#include <mutex>

class GameSimulation : public WorldInfoInterface {
public:
    explicit GameSimulation(jt::LoggerInterface& logger, UnitInfoCollection& unitInfos);
    void prepareSimulationForNewRound();
    void addUnit(ObjectProperties const& props);
    void performSimulation(SimulationResultMessageSender& sender);
    // TODO think about using an optional here, as there might not be a valid target.
    // TODO otherwise, just return the passed in position?
    std::weak_ptr<SimulationObjectInterface> getClosestTargetTo(
        const jt::Vector2f& pos, int playerId) override;
    float getLocalSpeedFactorAt(jt::Vector2f const& pos, jt::Vector2f const& dir) override;

private:
    jt::LoggerInterface& m_logger;
    UnitInfoCollection& m_unitInfos;

    std::shared_ptr<Terrain> m_world;
    std::vector<ObjectProperties> m_unitInformationForRoundStart;
    std::vector<std::shared_ptr<SimulationObjectInterface>> m_simulationObjects;

    bool checkIfUnitIsUnique(ObjectProperties const& newUnitProps);
};

#endif // JAMTEMPLATE_GAME_SIMULATION_HPP
