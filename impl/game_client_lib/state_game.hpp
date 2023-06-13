#ifndef GAME_STATE_GAME_HPP
#define GAME_STATE_GAME_HPP

#include <client_network_connection.hpp>
#include <client_placement_data.hpp>
#include <game_properties.hpp>
#include <game_state.hpp>
#include <map/terrain.hpp>
#include <object_group.hpp>
#include <object_properties.hpp>
#include <player_id_dispatcher.hpp>
#include <server_connection.hpp>
#include <simulation_result_data.hpp>
#include <terrain_renderer.hpp>
#include <unit.hpp>
#include <unit_info.hpp>
#include <unit_info_collection.hpp>
#include <unit_placement/placement_manager.hpp>
#include <unit_placement/unit_id_manager.hpp>
#include <vector.hpp>
#include <map>
#include <memory>
#include <vector>

// fwd decls
namespace jt {
class Shape;

class Sprite;

class Vignette;
} // namespace jt

class Hud;

enum class InternalState {
    WaitForAllPlayers,
    PlaceUnits,
    WaitForSimulationResults,
    Playback,
    EndWin,
    EndLose
};

class StateGame : public jt::GameState {
public:
    std::string getName() const override;

    void setConnection(std::shared_ptr<ClientNetworkConnection> connection, bool botAsPlayerZero,
        bool botAsPlayerOne);

private:
    std::shared_ptr<ClientNetworkConnection> m_connection { nullptr };
    std::shared_ptr<ServerConnection> m_serverConnection { nullptr };

    std::shared_ptr<jt::Vignette> m_vignette { nullptr };
    std::shared_ptr<Hud> m_hud { nullptr };
    std::shared_ptr<Terrain> m_world { nullptr };
    std::shared_ptr<TerrainRenderer> m_world_renderer { nullptr };

    std::shared_ptr<PlacementManager> m_placementManager { nullptr };
    std::shared_ptr<jt::ObjectGroup<Unit>> m_units { nullptr };

    mutable ClientPlacementData m_clientEndPlacementData;

    std::shared_ptr<UnitInfoCollection> m_unitInfo;

    mutable InternalState m_internalState { InternalState::WaitForAllPlayers };

    SimulationResultDataForAllFrames m_simulationResultsForAllFrames;

    std::shared_ptr<PlayerIdDispatcher> m_playerIdDispatcher;
    mutable std::shared_ptr<jt::Shape> m_arrowShape;

    mutable bool m_addBotAsPlayerZero { false };
    mutable bool m_addBotAsPlayerOne { false };

    bool m_running { true };
    bool m_hasEnded { false };

    int m_round { 1 };
    std::size_t m_tickId { 0 };
    std::map<int, int> m_playerHP { { 0, GP::InitialPlayerHP() }, { 1, GP::InitialPlayerHP() } };

    void onCreate() override;
    void onEnter() override;
    void onUpdate(float const elapsed) override;
    void onDraw() const override;
    void endGame();

    void createPlayer();

    void placeUnits(float elapsed);

    void playbackSimulation(float elapsed);

    void transitionWaitForPlayersToStartPlaceUnits();
    // const as it needs to be called from onDraw;
    void transitionPlaceUnitsToWaitForSimulationResults() const;
    void transitionWaitForSimulationResultsToPlayback();
    void transitionPlaybackToPlaceUnits();
    void placeUnitsForOneTick(
        SimulationResultDataForOneFrame const& propertiesForAllUnitsForThisTick);
    void resetAllUnits();
};

#endif
