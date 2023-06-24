#ifndef GAME_GAMEPROPERTIES_HPP
#define GAME_GAMEPROPERTIES_HPP

#include <color/color.hpp>
#include <color/color_factory.hpp>
#include <color/palette.hpp>
#include <vector.hpp>
#include <cstddef>
#include <map>
#include <memory>
#include <string>

class GP {
public:
    GP() = delete;

    static std::string GameName() { return "Medibellum"; }
    static std::string AuthorName() { return "Dr.Laguna & Semitalis"; }
    static std::string JamName() { return "Kajam-13"; }
    static std::string JamDate() { return "2023-06"; }
    static std::string ExplanationText() { return ""; }

    static jt::Vector2f GetWindowSize() { return jt::Vector2f { 1024, 800 }; }
    static float GetZoom() { return 2.0f; }
    static jt::Vector2f GetScreenSize() { return GetWindowSize() * (1.0f / GetZoom()); }

    static jt::Color PaletteBackground() { return GP::getPalette().getColor(5); }
    static jt::Color PaletteFontFront() { return GP::getPalette().getColor(0); }
    static jt::Color PalleteFrontHighlight() { return GP::getPalette().getColor(1); }
    static jt::Color PaletteFontShadow() { return GP::getPalette().getColor(6); }
    static jt::Color PaletteFontCredits() { return GP::getPalette().getColor(2); }

    static jt::Color ColorPlayer0() { return jt::ColorFactory::fromHexString("#5d7cd4"); }
    static jt::Color ColorPlayer1() { return jt::ColorFactory::fromHexString("#f57979"); }

    static char const* HpFontAssetPath() { return "assets/fonts/PorspicanSerif-Regular.otf"; }

    static jt::Palette getPalette();

    static std::size_t MaxNumberOfStepsPerRound();

    static jt::Vector2f UnitAnimationOffset();
    static int InitialPlayerHP();

    static int ZLayerUI();
    static std::map<std::string, int> animationPriorities();
};

#endif
