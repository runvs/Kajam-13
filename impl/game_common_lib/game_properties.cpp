#include "game_properties.hpp"
#include "color/palette_builder.hpp"

namespace {

jt::Palette createPalette()
{
    jt::PaletteBuilder builder;
    return builder
        .addColorsFromGPL(R"(GIMP Palette
#Palette Name: Dreamscape8
#Description: A palette made of low saturation colours to give your art a dreamlike quality.
#Colors: 8
201	204	161	c9cca1
202	160	90	caa05a
174	106	71	ae6a47
139	64	73	8b4049
84	51	68	543344
81	82	98	515262
99	120	125	63787d
142	160	145	8ea091
)")
        .create();
}

} // namespace

jt::Palette GP::getPalette()
{
    static auto const p = createPalette();
    return p;
}

std::size_t GP::MaxNumberOfStepsPerRound() { return 8000; }

jt::Vector2f GP::UnitAnimationOffset() { return jt::Vector2f { -9.0f, -16.0f }; }

int GP::InitialPlayerHP() { return 4000; }

int GP::ZLayerUI() { return 100; }

std::map<std::string, int> GP::animationPriorities()
{
    return {
        // clang-format off
            { "idle", 0 },
            { "damage", 1 },
            { "walk", 2 },
            { "attack", 2 },
            { "death", 3 }
        // clang-format on
    };
}

float GP::UnitExperienceForLevelUpExponent() { return 0.875f; }

std::string GP::GameName() { return "Medibellum"; }

std::string GP::AuthorName() { return "Adkiem & Dr.Laguna & Semitalis"; }

std::string GP::JamName() { return "Kajam-13"; }

std::string GP::JamDate() { return "2023-06"; }

std::string GP::ExplanationText() { return ""; }

jt::Vector2f GP::GetWindowSize() { return jt::Vector2f { 1024, 800 }; }

float GP::GetZoom() { return 2.0f; }

jt::Vector2f GP::GetScreenSize() { return GetWindowSize() * (1.0f / GetZoom()); }

jt::Color GP::PaletteBackground() { return GP::getPalette().getColor(5); }

jt::Color GP::PaletteFontFront() { return GP::getPalette().getColor(0); }

jt::Color GP::PaletteFontShadow() { return jt::Color { 50, 50, 50, 100 }; }

jt::Color GP::PaletteFontCredits() { return GP::getPalette().getColor(2); }

jt::Color GP::ColorPlayer0() { return jt::ColorFactory::fromHexString("#5d7cd4"); }

jt::Color GP::ColorPlayer1() { return jt::ColorFactory::fromHexString("#f57979"); }

float GP::TimePerSimulationUpdate() { return 0.005f; }
