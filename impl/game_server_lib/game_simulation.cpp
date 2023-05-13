#include "game_simulation.hpp"
#include "object_properties.hpp"
#include "vector.hpp"
#include <game_properties.hpp>
#include <server_unit.hpp>
#include <memory>
GameSimulation::GameSimulation(jt::LoggerInterface& logger)
    : m_logger { logger }
{
}

void GameSimulation::updateSimulationForNewRound(std::map<int, PlayerInfo> const& playerData)
{
    m_latestPlayerData = playerData;
    m_simulationObjects.clear();
    for (auto const& kvp : m_latestPlayerData) {
        for (auto const& props : kvp.second.roundEndPlacementData.m_properties) {
            auto obj = std::make_unique<ServerUnit>();
            jt::Vector2f const pos { props.floats.at("posX"), props.floats.at("posY") };
            obj->setUnitID(props.ints.at("unitID"));
            obj->setPosition(pos);
            m_simulationObjects.emplace_back(std::move(obj));
        }
    }
}

void GameSimulation::performSimulation(ServerNetworkConnection& connection)
{
    auto const timePerUpdate = 0.005f;

    for (auto i = 0u; i != GP::NumberOfStepsPerRound(); ++i) {
        std::vector<ObjectProperties> props;
        for (auto& obj : m_simulationObjects) {
            obj->update(timePerUpdate);

            auto data = obj->saveState();
            data.ints["simulationTick"] = i;
            props.push_back(data);
        }
        Message m;
        m.type = MessageType::SimulationResult;
        nlohmann::json j = props;
        m.data = j.dump();
        // TODO do not send one message per tick but only one per full round
        for (auto const& kvp : m_latestPlayerData) {
            connection.sendMessage(m, kvp.second.endpoint);
        }

        // TODO sleep seems to be necessary otherwise client will get hickups (because of race
        // condition?)
        std::this_thread::sleep_for(std::chrono::milliseconds { 50 });
    }

    m_logger.info("simulation finished");
}
