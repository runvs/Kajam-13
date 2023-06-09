﻿#include "state_menu.hpp"
#include <build_info.hpp>
#include <color/color.hpp>
#include <compression/compressor_impl.hpp>
#include <compression/logging_compressor.hpp>
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <input/input_manager.hpp>
#include <lerp.hpp>
#include <log/license_info.hpp>
#include <math_helper.hpp>
#include <screeneffects/vignette.hpp>
#include <state_game.hpp>
#include <state_manager/state_manager_transition_fade_to_black.hpp>
#include <text.hpp>
#include <tweens/tween_alpha.hpp>
#include <tweens/tween_color.hpp>
#include <tweens/tween_position.hpp>

void StateMenu::onCreate()
{
    createMenuText();
    createShapes();
    createVignette();

    add(std::make_shared<jt::LicenseInfo>());

    getGame()->stateManager().setTransition(std::make_shared<jt::StateManagerTransitionFadeToBlack>(
        GP::GetScreenSize(), textureManager()));
    m_compressor = std::make_shared<LoggingCompressor>(
        getGame()->logger(), std::make_shared<CompressorImpl>());
    m_connector = std::make_shared<ServerConnector>(m_compressor);
    add(m_connector);

    auto bgmMenu = getGame()->audio().getPermanentSound("bgm_menu");
    if (bgmMenu == nullptr) {
        bgmMenu = getGame()->audio().addPermanentSound("bgm_menu", "assets/sfx/menu.ogg");
    }
    bgmMenu->setLoop(true);
    bgmMenu->play();
    getGame()->audio().fades().volumeFade(bgmMenu, 0.5f, bgmMenu->getVolume(), 1.0f);

    auto bgm_main = getGame()->audio().getPermanentSound("bgm_main");
    if (bgm_main) {
        getGame()->audio().fades().volumeFade(bgm_main, 0.5f, bgm_main->getVolume(), 0.0f);
    }
}

void StateMenu::onEnter()
{
    createTweens();
    m_started = false;
}

void StateMenu::createVignette()
{
    m_vignette = std::make_shared<jt::Vignette>(GP::GetScreenSize());
    add(m_vignette);
}

void StateMenu::createShapes()
{
    m_background
        = jt::dh::createShapeRect(GP::GetScreenSize(), GP::PaletteBackground(), textureManager());
    m_overlay = jt::dh::createShapeRect(GP::GetScreenSize(), jt::colors::Black, textureManager());
}

void StateMenu::createMenuText()
{
    createTextTitle();
    createTextExplanation();
    createTextCredits();
}

void StateMenu::createTextExplanation()
{
    m_textExplanation
        = jt::dh::createText(renderTarget(), GP::ExplanationText(), 16U, GP::PaletteFontFront());
    auto const half_width = GP::GetScreenSize().x / 2.0f;
    m_textExplanation->setPosition({ half_width, 180 });
    m_textExplanation->setShadow(GP::PaletteFontShadow(), jt::Vector2f { 2, 2 });
}

void StateMenu::createTextCredits()
{
    m_textCredits = jt::dh::createText(renderTarget(),
        "Created by " + GP::AuthorName() + " for " + GP::JamName() + "\n" + GP::JamDate()
            + "\n\nF9 for License Information",
        16U, GP::PaletteFontCredits());
    m_textCredits->setTextAlign(jt::Text::TextAlign::LEFT);
    m_textCredits->setPosition({ 10, GP::GetScreenSize().y - 70 });
    m_textCredits->setShadow(GP::PaletteFontShadow(), jt::Vector2f { 1, 1 });

    m_textVersion = jt::dh::createText(renderTarget(), "", 16U, GP::PaletteFontCredits());
    if (jt::BuildInfo::gitTagName() != "") {
        m_textVersion->setText(jt::BuildInfo::gitTagName());
    } else {
        m_textVersion->setText(
            jt::BuildInfo::gitCommitHash().substr(0, 6) + " " + jt::BuildInfo::timestamp());
    }
    m_textVersion->setTextAlign(jt::Text::TextAlign::RIGHT);
    m_textVersion->setPosition({ GP::GetScreenSize().x - 5.0f, GP::GetScreenSize().y - 20.0f });
    m_textVersion->setShadow(GP::PaletteFontShadow(), jt::Vector2f { 1, 1 });
}

void StateMenu::createTextTitle()
{
    float half_width = GP::GetScreenSize().x / 2;
    m_textTitle = jt::dh::createText(renderTarget(), GP::GameName(), 48U, GP::PaletteFontFront());
    m_textTitle->setPosition({ half_width, 20 });
    m_textTitle->setShadow(GP::PaletteFontShadow(), jt::Vector2f { 4, 4 });
}

void StateMenu::createTweens()
{
    createTweenOverlayAlpha();
    createTweenTitleAlpha();
    createTweenCreditsPosition();
}

void StateMenu::createTweenTitleAlpha()
{
    auto tween = jt::TweenAlpha::create(m_textTitle, 0.55f, 0, 255);
    tween->setStartDelay(0.2f);
    tween->setSkipFrames();
    add(tween);
}

void StateMenu::createTweenOverlayAlpha()
{
    auto tween = jt::TweenAlpha::create(m_overlay, 0.5f, std::uint8_t { 255 }, std::uint8_t { 0 });
    tween->setSkipFrames();
    add(tween);
}

void StateMenu::createTweenCreditsPosition()
{
    auto creditsPositionStart = m_textCredits->getPosition() + jt::Vector2f { 0, 150 };
    auto creditsPositionEnd = m_textCredits->getPosition();

    auto tweenCredits
        = jt::TweenPosition::create(m_textCredits, 0.75f, creditsPositionStart, creditsPositionEnd);
    tweenCredits->setStartDelay(0.8f);
    tweenCredits->setSkipFrames();
    add(tweenCredits);

    auto versionPositionStart = m_textVersion->getPosition() + jt::Vector2f { 0, 150 };
    auto versionPositionEnd = m_textVersion->getPosition();
    auto tweenVersion
        = jt::TweenPosition::create(m_textVersion, 0.75f, versionPositionStart, versionPositionEnd);
    tweenVersion->setStartDelay(0.8f);
    tweenVersion->setSkipFrames();
    add(tweenVersion);
}

void StateMenu::onUpdate(float const elapsed) { updateDrawables(elapsed); }

void StateMenu::updateDrawables(const float& elapsed)
{
    m_background->update(elapsed);
    m_textTitle->update(elapsed);
    m_textExplanation->update(elapsed);
    m_textCredits->update(elapsed);
    m_textVersion->update(elapsed);
    m_overlay->update(elapsed);
    m_vignette->update(elapsed);
}

void StateMenu::onDraw() const
{
    m_background->draw(renderTarget());

    m_textTitle->draw(renderTarget());
    m_textExplanation->draw(renderTarget());
    m_textCredits->draw(renderTarget());
    m_textVersion->draw(renderTarget());
    m_overlay->draw(renderTarget());
    m_vignette->draw();
}

std::string StateMenu::getName() const { return "State Menu"; }
