#ifndef GAME_STATE_GAME_HPP
#define GAME_STATE_GAME_HPP

#include "client_network_connection.hpp"
#include "server_connection.hpp"
#include "vector.hpp"
#include <box2dwrapper/box2d_world_interface.hpp>
#include <client_end_placement_data.hpp>
#include <game_state.hpp>
#include <memory>
#include <vector>

// fwd decls
namespace jt {
class Shape;

class Sprite;

class Vignette;
} // namespace jt

class Hud;

class StateGame : public jt::GameState {
public:
    std::string getName() const override;

    void setConnection(std::shared_ptr<ClientNetworkConnection> connection);

private:
    std::shared_ptr<ClientNetworkConnection> m_connection { nullptr };
    std::shared_ptr<ServerConnection> m_serverConnection { nullptr };

    std::shared_ptr<jt::Shape> m_background { nullptr };
    std::shared_ptr<jt::Vignette> m_vignette { nullptr };
    std::shared_ptr<Hud> m_hud { nullptr };
    std::shared_ptr<jt::Box2DWorldInterface> m_world { nullptr };

    // TODO replace with vector of unit positions later
    ClientEndPlacementData m_clientEndPlacementData;

    bool m_running { true };
    bool m_hasEnded { false };

    int m_round { 1 };

    void onCreate() override;
    void onEnter() override;
    void onUpdate(float const elapsed) override;
    void onDraw() const override;

    void endGame();

    void createPlayer();
};

#endif
