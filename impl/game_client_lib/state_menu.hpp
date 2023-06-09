﻿#ifndef GAME_STATE_MENU_HPP
#define GAME_STATE_MENU_HPP

#include <compression/compressor_interface.hpp>
#include <game_state.hpp>
#include <server_connector.hpp>
#include <memory>

namespace jt {
class Text;
class Shape;
class Animation;
class Sprite;
class Vignette;
} // namespace jt

class StateMenu : public jt::GameState {
public:
    std::string getName() const override;

    void setScore(int s) { m_score = s; };

private:
    std::shared_ptr<jt::Shape> m_background;

    std::shared_ptr<jt::Text> m_textTitle;
    std::shared_ptr<jt::Text> m_textExplanation;
    std::shared_ptr<jt::Text> m_textCredits;
    std::shared_ptr<jt::Text> m_textVersion;

    std::shared_ptr<jt::Shape> m_overlay;
    std::shared_ptr<jt::Vignette> m_vignette;

    std::shared_ptr<CompressorInterface> m_compressor;
    std::shared_ptr<ServerConnector> m_connector;

    int m_score { 0 };

    bool m_started { false };

    void onCreate() override;

    void onEnter() override;

    void createVignette();

    void createShapes();

    void createMenuText();

    void createTextCredits();

    void createTextTitle();

    void createTweens();

    void createTweenTitleAlpha();

    void createTweenOverlayAlpha();

    void createTweenCreditsPosition();

    void onUpdate(float const elapsed) override;

    void updateDrawables(const float& elapsed);

    void onDraw() const override;

    void createTextExplanation();
};

#endif
