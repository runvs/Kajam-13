#ifndef JAMTEMPLATE_END_LOSE_HPP
#define JAMTEMPLATE_END_LOSE_HPP

#include <internal_state/internal_state_interface.hpp>
#include <sprite.hpp>
#include <memory>

class EndLose : public InternalStateInterface {
public:
    void create(StateGame& state) override;
    void update(StateGame& state, float elapsed) override;
    void draw(StateGame& state) override;

private:
    std::shared_ptr<jt::Sprite> m_imageDefeat;
    std::shared_ptr<jt::Sprite> m_imageBackToMenu;
};

#endif // JAMTEMPLATE_END_LOSE_HPP
