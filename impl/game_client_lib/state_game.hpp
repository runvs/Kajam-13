#ifndef GAME_STATE_GAME_HPP
#define GAME_STATE_GAME_HPP

#include <client_network_connection.hpp>
#include <client_placement_data.hpp>
#include <critters/critter.hpp>
#include <game_properties.hpp>
#include <game_state.hpp>
#include <internal_state/internal_state_interface.hpp>
#include <internal_state/internal_state_manager.hpp>
#include <map/terrain.hpp>
#include <object_group.hpp>
#include <object_properties.hpp>
#include <particle_system.hpp>
#include <player_id_dispatcher.hpp>
#include <screeneffects/clouds.hpp>
#include <server_connection.hpp>
#include <simulation_result_data.hpp>
#include <terrain_renderer.hpp>
#include <text.hpp>
#include <unit.hpp>
#include <unit_info.hpp>
#include <unit_info_collection.hpp>
#include <unit_placement/placement_manager.hpp>
#include <unit_placement/unit_id_manager.hpp>
#include <vector.hpp>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

// fwd decls
namespace jt {
class Shape;

class Sprite;

class Vignette;
} // namespace jt

class Hud;
class InternalStateInterface;

class StateGame : public jt::GameState {
public:
    std::string getName() const override;

    void setConnection(std::shared_ptr<ClientNetworkConnection> connection, bool botAsPlayerZero,
        bool botAsPlayerOne);

    std::shared_ptr<InternalStateManager> getStateManager();
    std::shared_ptr<ServerConnection> getServerConnection();
    std::shared_ptr<TerrainRenderer> getTerrainRenderer();
    std::shared_ptr<PlacementManager> getPlacementManager();
    std::shared_ptr<UnitInfoCollection> getUnitInfo();
    std::shared_ptr<jt::ObjectGroup<Unit>> getUnits();
    std::shared_ptr<jt::ObjectGroup<Critter>> getCritters();

    // TODO move to InternalStateManager
    void transitionWaitForPlayersToSelectStartingUnits();
    void transitionWaitForSimulationResultsToPlayback();
    void transitionPlaybackToPlaceUnits();

    void playbackSimulation(float elapsed);
    void drawArrows() const;

    int getRound();
    void flashUnitsForUpgrade(std::string const& unitType);

private:
    std::shared_ptr<ClientNetworkConnection> m_connection { nullptr };
    std::shared_ptr<ServerConnection> m_serverConnection { nullptr };

    std::shared_ptr<jt::Vignette> m_vignette { nullptr };
    std::shared_ptr<Hud> m_hud { nullptr };
    std::shared_ptr<Terrain> m_world { nullptr };
    std::shared_ptr<TerrainRenderer> m_terrainRenderer { nullptr };

    std::shared_ptr<PlacementManager> m_placementManager { nullptr };
    std::shared_ptr<jt::ObjectGroup<Unit>> m_units { nullptr };
    std::shared_ptr<jt::Clouds> m_clouds { nullptr };

    std::shared_ptr<jt::ObjectGroup<Critter>> m_critters { nullptr };

    std::shared_ptr<UnitInfoCollection> m_unitInfo { nullptr };

    std::shared_ptr<InternalStateManager> m_internalStateManager { nullptr };

    SimulationResultDataForAllFrames m_simulationResultsForAllFrames;

    std::shared_ptr<PlayerIdDispatcher> m_playerIdDispatcher { nullptr };
    mutable std::shared_ptr<jt::Shape> m_arrowShape { nullptr };

    std::shared_ptr<jt::Sprite> m_stateIconWaiting { nullptr };
    std::shared_ptr<jt::Sprite> m_stateIconPlacing { nullptr };
    std::shared_ptr<jt::Sprite> m_stateIconFighting { nullptr };

    std::shared_ptr<jt::Text> m_textRound { nullptr };
    std::shared_ptr<jt::Text> m_textPlayerZeroHp { nullptr };
    std::shared_ptr<jt::Text> m_textPlayerOneHp { nullptr };

    mutable bool m_addBotAsPlayerZero { false };
    mutable bool m_addBotAsPlayerOne { false };

    bool m_running { true };
    bool m_hasEnded { false };

    int m_round { 1 };
    std::size_t m_tickId { 0 };
    std::map<int, int> m_playerHP { { 0, GP::InitialPlayerHP() }, { 1, GP::InitialPlayerHP() } };

    std::shared_ptr<jt::ParticleSystem<jt::Shape, 50>> m_explosionParticles { nullptr };

    // at end of round
    float m_playbackOverflowTime = 2.5f;

    void onCreate() override;
    void onEnter() override;
    void onUpdate(float const elapsed) override;
    void onDraw() const override;

    void endGame();

    void playbackOneFrame(SimulationResultDataForOneFrame const& currentFrame);

    std::shared_ptr<Unit> findOrCreateUnit(int pid, int uid, std::string const& type);

    void resetAllUnits();
};

#endif
