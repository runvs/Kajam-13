#ifndef GAME_STATE_GAME_HPP
#define GAME_STATE_GAME_HPP

#include "unit_placement/unit_id_manager.hpp"
#include <box2dwrapper/box2d_world_interface.hpp>
#include <client_end_placement_data.hpp>
#include <client_network_connection.hpp>
#include <game_state.hpp>
#include <object_group.hpp>
#include <object_properties.hpp>
#include <player_id_dispatcher.hpp>
#include <server_connection.hpp>
#include <unit.hpp>
#include <unit_info.hpp>
#include <unit_info_collection.hpp>
#include <unit_placement/placement_manager.hpp>
#include <vector.hpp>
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
};

class StateGame : public jt::GameState {
public:
    std::string getName() const override;

    void setConnection(std::shared_ptr<ClientNetworkConnection> connection, bool botAsPlayerZero,
        bool botAsPlayerOne);

private:
    std::shared_ptr<ClientNetworkConnection> m_connection { nullptr };
    std::shared_ptr<ServerConnection> m_serverConnection { nullptr };

    std::shared_ptr<jt::Shape> m_background { nullptr };
    std::shared_ptr<jt::Vignette> m_vignette { nullptr };
    std::shared_ptr<Hud> m_hud { nullptr };
    std::shared_ptr<jt::Box2DWorldInterface> m_world { nullptr };

    std::shared_ptr<PlacementManager> m_placementManager { nullptr };
    std::shared_ptr<jt::ObjectGroup<Unit>> m_units { nullptr };

    mutable ClientEndPlacementData m_clientEndPlacementData;

    std::shared_ptr<UnitInfoCollection> m_unitInfo;

    mutable InternalState m_internalState { InternalState::WaitForAllPlayers };

    // TODO could be converted into a class?
    std::vector<std::vector<ObjectProperties>> m_properties;

    std::shared_ptr<PlayerIdDispatcher> m_playerIdDispatcher;

    mutable bool m_addBotAsPlayerZero { false };
    mutable bool m_addBotAsPlayerOne { false };

    bool m_running { true };
    bool m_hasEnded { false };

    int m_round { 1 };
    std::size_t m_tickId { 0 };

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
        std::vector<ObjectProperties> const& propertiesForAllUnitsForThisTick);
    void resetAllUnits();
};

#endif
