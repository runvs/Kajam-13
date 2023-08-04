#ifndef JAMTEMPLATE_PLAYBACK_HPP
#define JAMTEMPLATE_PLAYBACK_HPP

#include <internal_state/internal_state_interface.hpp>

class Playback : public InternalStateInterface {
public:
    void create(StateGame& state) override;
    void update(StateGame& state, float elapsed) override;
    void draw(StateGame& state) override;
};

#endif // JAMTEMPLATE_PLAYBACK_HPP
