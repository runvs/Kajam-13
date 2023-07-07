#ifndef GAME_GAMEPROPERTIES_HPP
#define GAME_GAMEPROPERTIES_HPP

#include <color/color.hpp>
#include <color/color_factory.hpp>
#include <color/palette.hpp>
#include <vector2.hpp>
#include <cstddef>
#include <map>
#include <memory>
#include <string>

class GP {
public:
    GP() = delete;

    static std::string GameName();

    static std::string AuthorName();

    static std::string JamName();

    static std::string JamDate();

    static std::string ExplanationText();

    static jt::Vector2f GetWindowSize();

    static float GetZoom();

    static jt::Vector2f GetScreenSize();

    static jt::Color PaletteBackground();

    static jt::Color PaletteFontFront();

    static jt::Color PaletteFontShadow();

    static jt::Color PaletteFontCredits();

    static jt::Color ColorPlayer0();

    static jt::Color ColorPlayer1();

    static jt::Palette getPalette();

    static std::size_t MaxNumberOfStepsPerRound();

    static float TimePerSimulationUpdate();

    static jt::Vector2f UnitAnimationOffset();
    static int InitialPlayerHP();

    static int ZLayerUI();
    static std::map<std::string, int> animationPriorities();
    static float UnitExperienceForLevelUpExponent();
};

float convertSlopeToSpeedFactor(float slope);

#endif
