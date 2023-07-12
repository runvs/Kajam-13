#include "state_game.hpp"
#include <color/color.hpp>
#include <critters/bird.hpp>
#include <critters/bunny.hpp>
#include <critters/deer.hpp>
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <graphics/drawable_interface.hpp>
#include <input/keyboard/keyboard_defines.hpp>
#include <internal_state/internal_state_manager.hpp>
#include <json_keys.hpp>
#include <math_helper.hpp>
#include <message.hpp>
#include <network_data/select_unit_info.hpp>
#include <network_data/unit_info_collection.hpp>
#include <object_group.hpp>
#include <particle_system.hpp>
#include <player_id_dispatcher.hpp>
#include <random/random.hpp>
#include <screeneffects/vignette.hpp>
#include <server_connection.hpp>
#include <shape.hpp>
#include <simulation_result_data.hpp>
#include <state_menu.hpp>
#include <system_helper.hpp>
#include <tweens/tween_alpha.hpp>
#include <tweens/tween_position.hpp>
#include <unit_placement/placement_manager.hpp>
#include <vector2.hpp>
#include <imgui.h>
#include <memory>

void StateGame::onCreate()
{
    getGame()->gfx().createZLayer(GP::ZLayerUI());
    m_terrain = std::make_shared<Terrain>("assets/maps/map_load.json");
    m_terrainRenderer = std::make_shared<TerrainRenderer>(*m_terrain);
    add(m_terrainRenderer);

    m_internalStateManager = std::make_shared<InternalStateManager>();

    m_units = std::make_shared<jt::ObjectGroup<Unit>>();
    add(m_units);

    m_vignette = std::make_shared<jt::Vignette>(GP::GetScreenSize());
    add(m_vignette);

    // StateGame will call drawObjects itself.
    setAutoDraw(false);

    if (m_addBotAsPlayerZero) {
        Message m;
        m.type = MessageType::AddBot;
        m_connection->sendMessage(m);
    }
    m_serverConnection = std::make_shared<ServerConnection>(getGame()->logger());
    m_serverConnection->setOnServerInfo([this](auto const& j) { updateServerInfo(j); });
    m_serverConnection->setConnection(m_connection);
    add(m_serverConnection);
    if (m_addBotAsPlayerOne) {
        if (m_addBotAsPlayerZero) {
            getGame()->logger().warning("Create game with two bots", { "StateGame", "Bots" });
        }
        Message m;
        m.type = MessageType::AddBot;
        m_connection->sendMessage(m);
    }

    m_arrowShape = jt::dh::createShapeCircle(1, jt::colors::White, textureManager());
    m_arrowShape->setOffset(jt::OffsetMode::CENTER);

    m_clouds = std::make_shared<jt::Clouds>(jt::Vector2f { 4.0f, 2.0f });
    add(m_clouds);

    m_critters = std::make_shared<jt::ObjectGroup<Critter>>();

    auto const placeCritterFn
        = [this]<typename CritterT>(float lowerbound, float upperbound, int count) {
              for (auto i = 0; i != count; ++i) {
                  for (auto x = 0; x != 10; ++x) {
                      auto const tmpPos = jt::Random::getRandomPointIn(GP::GetScreenSize());
                      auto const tmpHeight = m_terrain->getFieldHeight(tmpPos);
                      if (tmpHeight >= lowerbound && tmpHeight <= upperbound) {
                          auto c = std::make_shared<CritterT>();
                          c->setPosition(tmpPos);
                          add(c);
                          m_critters->push_back(c);
                          break;
                      }
                  }
              }
          };

    placeCritterFn.template operator()<Bird>(0.0f, 4.4f, jt::Random::getInt(5, 10));
    placeCritterFn.template operator()<Bunny>(1.0f, 2.4f, jt::Random::getInt(5, 10));
    placeCritterFn.template operator()<Deer>(1.0f, 2.4f, jt::Random::getInt(2, 6));

    m_explosionParticles = jt::ParticleSystem<jt::Shape, 150>::createPS(
        [this]() {
            std::shared_ptr<jt::Shape> shape
                = jt::dh::createShapeCircle(4, jt::colors::White, textureManager());
            shape->setPosition(jt::Vector2f { -5000.0f, -5000.0f });
            return shape;
        },
        [this](auto shape, auto const& pos) {
            shape->setPosition(jt::Random::getRandomPointInCircle(40.0f) + pos);
            auto const tw = jt::TweenAlpha::create(shape, 0.5f, 255, 0);
            add(tw);
        });
    add(m_explosionParticles);

    m_stateIconWaiting = std::make_shared<jt::Sprite>(
        "assets/images/states/waiting.png", jt::Recti { 0, 0, 183, 256 }, textureManager());
    m_stateIconPlacing = std::make_shared<jt::Sprite>(
        "assets/images/states/placing.png", jt::Recti { 0, 0, 183, 256 }, textureManager());
    m_stateIconFighting = std::make_shared<jt::Sprite>(
        "assets/images/states/fighting.png", jt::Recti { 0, 0, 183, 256 }, textureManager());

    m_textRound = jt::dh::createText(renderTarget(), "Round " + std::to_string(m_round), 14);
    m_textRound->setPosition({ GP::GetScreenSize().x / 2, 0 });
    m_textRound->setShadow(GP::PaletteFontShadow(), jt::Vector2f { 0, 1 });

    m_textTimeIndicator = jt::dh::createText(renderTarget(), "30.0", 16);
    m_textTimeIndicator->setPosition({ GP::GetScreenSize().x / 2, 48 });
    m_textTimeIndicator->setShadow(GP::PaletteFontShadow(), jt::Vector2f { 0, 1 });

    m_textPlayerZeroHp = jt::dh::createText(renderTarget(),
        std::to_string(GP::InitialPlayerHP()).c_str(), 22, GP::ColorPlayer0(), "assets/font.ttf");
    m_textPlayerZeroHp->setTextAlign(jt::Text::TextAlign::LEFT);
    m_textPlayerZeroHp->setPosition({ 8, 0 });
    m_textPlayerZeroHp->setShadow(GP::PaletteFontShadow(), jt::Vector2f { 1, 1 });
    m_textPlayerOneHp = jt::dh::createText(renderTarget(),
        std::to_string(GP::InitialPlayerHP()).c_str(), 22, GP::ColorPlayer1(), "assets/font.ttf");
    m_textPlayerOneHp->setTextAlign(jt::Text::TextAlign::RIGHT);
    m_textPlayerOneHp->setPosition({ GP::GetScreenSize().x - 10, 0 });
    m_textPlayerOneHp->setShadow(GP::PaletteFontShadow(), jt::Vector2f { -1, 1 });

    auto bgm = getGame()->audio().getPermanentSound("bgm_main");
    if (!bgm) {
        bgm = getGame()->audio().addPermanentSound("bgm_main", "assets/sfx/main_theme.ogg");
    }
    bgm->setLoop(true);
    bgm->play();
    getGame()->audio().fades().volumeFade(bgm, 0.5f, bgm->getVolume(), 1.0f);

    auto bgm_menu = getGame()->audio().getPermanentSound("bgm_menu");
    if (bgm_menu) {
        getGame()->audio().fades().volumeFade(bgm_menu, 0.5f, bgm_menu->getVolume(), 0.0f);
    }
}

void StateGame::onEnter() { }

void StateGame::onUpdate(float const elapsed)
{
    if (m_running) {
        m_explosionParticles->update(elapsed);
        // update game logic here

        if (getGame()->input().keyboard()->pressed(jt::KeyCode::LShift)
            && getGame()->input().keyboard()->pressed(jt::KeyCode::Escape)) {
            endGame();
        }

        m_internalStateManager->getActiveState()->update(*this, elapsed);
    }

    m_textRound->update(elapsed);
    m_textTimeIndicator->update(elapsed);
    m_textPlayerZeroHp->update(elapsed);
    m_textPlayerOneHp->update(elapsed);
}

void StateGame::playbackSimulation(float elapsed)
{
    if (!m_simulationResultsForAllFrames.allFrames.empty()) {
        auto const maxTicks = m_simulationResultsForAllFrames.allFrames.size();
        auto const maxTime = GP::TimePerSimulationUpdate() * maxTicks;
        auto const percentage = (maxTicks - m_tickId) * 1.0f / maxTicks;
        m_textTimeIndicator->setText(
            jt::MathHelper::floatToStringWithXDigits((percentage * maxTime), 1));
        if (m_tickId < m_simulationResultsForAllFrames.allFrames.size() - 1) {
            m_tickId++;
            auto const& propertiesForAllUnitsForThisFrame
                = m_simulationResultsForAllFrames.allFrames.at(m_tickId);
            playbackOneFrame(propertiesForAllUnitsForThisFrame);
        } else {
            m_playbackOverflowTime -= elapsed;
            if (m_playbackOverflowTime <= 0) {
                m_playbackOverflowTime = 2.5f;
                getStateManager()->switchToState(InternalState::PlaceUnits, *this);
            }
        }
    }
}

void StateGame::playbackOneFrame(SimulationResultDataForOneFrame const& currentFrame)
{
    if (!currentFrame.m_playerHP.empty()) {
        getGame()->logger().info("new player hp received");
        m_playerHP = currentFrame.m_playerHP;
        auto const thisPlayerId = m_serverConnection->getPlayerId();
        auto const otherPlayerId = ((thisPlayerId == 0) ? 1 : 0);
        m_textPlayerZeroHp->setText(std::to_string(m_playerHP.at(0)));
        m_textPlayerOneHp->setText(std::to_string(m_playerHP.at(1)));
        if (m_playerHP.at(thisPlayerId) <= 0) {
            getStateManager()->switchToState(InternalState::EndLose, *this);
        } else if (m_playerHP.at(otherPlayerId) <= 0) {
            getStateManager()->switchToState(InternalState::EndWin, *this);
        }
    }

    for (auto const& propsForOneUnit : currentFrame.m_units) {
        std::shared_ptr<Unit> foundUnit = findOrCreateUnit(
            propsForOneUnit.playerID, propsForOneUnit.unitID, propsForOneUnit.unitType);
        foundUnit->updateState(propsForOneUnit);
    }

    for (auto const& expl : currentFrame.m_explosions) {
        getGame()->gfx().camera().shake(0.5f, 5);
        m_explosionParticles->fire(20, expl.position);
    }

    for (auto const& shield : currentFrame.m_shields) {

        auto const ids = std::make_pair(shield.playerID, shield.unitID);
        if (m_shieldParticles.count(ids) == 0) {
            m_shieldParticles[ids] = jt::ParticleSystem<jt::Shape, 40>::createPS(
                [this]() {
                    std::shared_ptr<jt::Shape> shape = jt::dh::createShapeRect(
                        jt::Vector2f { 2.0f, 2.0f }, jt::colors::White, textureManager());
                    shape->setPosition(jt::Vector2f { -5000.0f, -5000.0f });
                    return shape;
                },
                [this, radius = shield.radius, pid = shield.playerID](auto shape, auto const& pos) {
                    auto const startPos = jt::Random::getRandomPointOnCircle(radius) + pos;
                    shape->setPosition(startPos);

                    if (jt::Random::getChance()) {
                        if (pid == 0) {
                            shape->setColor(GP::ColorPlayer0());
                        } else {
                            shape->setColor(GP::ColorPlayer1());
                        }
                    } else {
                        shape->setColor(jt::colors::White);
                    }
                    auto const tw = jt::TweenAlpha::create(shape, 0.5f, 255, 0);
                    add(tw);
                    auto twp = jt::TweenPosition::create(shape, 0.3f, startPos, pos);
                    twp->setStartDelay(0.2f);
                    add(twp);
                });

            add(m_shieldParticles[ids]);
        }

        if (shield.hpCurrent <= 0) {
            continue;
        }
        m_shieldParticles.at(ids)->fire(2, shield.pos);
        m_shieldParticles.at(ids)->update(0.0f);
    }
}

std::shared_ptr<Unit> StateGame::findOrCreateUnit(int pid, int uid, const std::string& type)
{
    for (auto const& u : *m_units) {
        auto currentUnit = u.lock();
        if (!currentUnit) {
            continue;
        }
        if (currentUnit->getPlayerID() != pid) {
            continue;
        }
        if (currentUnit->getUnitID() != uid) {
            continue;
        }
        return currentUnit;
    }

    // Spawn a new  unit
    std::shared_ptr<Unit> newUnit = std::make_shared<Unit>(m_unitInfo->getInfoForType(type));
    m_units->push_back(newUnit);
    add(newUnit);
    newUnit->setIDs(uid, pid);
    return newUnit;
}

void StateGame::transitionWaitForPlayersToSelectStartingUnits()
{
    m_playerIdDispatcher = std::make_shared<PlayerIdDispatcher>(m_serverConnection->getPlayerId());
    m_unitInfo = std::make_shared<UnitInfoCollection>(
        getGame()->logger(), m_serverConnection->getUnitInfo());

    m_placementManager = std::make_shared<PlacementManager>(
        m_terrain, m_serverConnection->getPlayerId(), m_playerIdDispatcher, m_unitInfo);
    add(m_placementManager);

    m_startingUnits = m_serverConnection->getStartingUnits();
}

void StateGame::transitionWaitForSimulationResultsToPlayback()
{
    m_simulationResultsForAllFrames = m_serverConnection->getRoundData();
    m_tickId = 0;
}

void StateGame::transitionPlaybackToPlaceUnits()
{
    m_tickId = 0;
    resetAllUnits();
    m_round++;
    m_textRound->setText("Round " + std::to_string(m_round));
    getGame()->logger().info("finished playing round simulation", { "StateGame" });
}

void StateGame::onDraw() const
{
    m_terrainRenderer->draw();

    // first draw all dead units
    for (auto const& u : *m_units) {
        auto const lockedUnit = u.lock();
        if (lockedUnit && !lockedUnit->isUnitAlive()) {
            lockedUnit->draw();
        }
    }

    for (auto const& c : *m_critters) {
        c.lock()->draw();
    }

    // then draw all alive units
    for (auto const& u : *m_units) {
        auto const lockedUnit = u.lock();
        if (lockedUnit && lockedUnit->isUnitAlive()) {
            lockedUnit->update(0.0f);
            lockedUnit->draw();
        }
    }

    // ouch
    m_internalStateManager->getActiveState()->draw(const_cast<StateGame&>(*this));

    m_explosionParticles->draw();
    for (auto const& kvp : m_shieldParticles) {
        kvp.second->draw();
    }
    m_clouds->draw();
    m_vignette->draw();

    ImGuiWindowFlags window_flags { ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoScrollWithMouse };
    ImGui::SetNextWindowPos({ GP::GetWindowSize().x / 2 - 23, 38 }, ImGuiCond_Always);
    ImGui::SetNextWindowSize({ 46, 64 }, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
    ImGui::Begin("State", nullptr, window_flags);
    ImGui::PopStyleVar(2);
    if (m_internalStateManager->getActiveStateE() == InternalState::WaitForAllPlayers) {
        ImGui::Image(jt::SystemHelper::nativeHandleToImTextureId<ImTextureID>(
                         m_stateIconWaiting->getSFSprite().getTexture()->getNativeHandle()),
            { 46.0f, 64.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f });
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Waiting for players to join");
            ImGui::EndTooltip();
        }
    } else if (m_internalStateManager->getActiveStateE()
        == InternalState::WaitForSimulationResults) {
        ImGui::Image(jt::SystemHelper::nativeHandleToImTextureId<ImTextureID>(
                         m_stateIconWaiting->getSFSprite().getTexture()->getNativeHandle()),
            { 46.0f, 64.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f });
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Waiting for other players to end unit placement");
            ImGui::EndTooltip();
        }
    } else if (m_internalStateManager->getActiveStateE() == InternalState::PlaceUnits) {
        ImGui::Image(jt::SystemHelper::nativeHandleToImTextureId<ImTextureID>(
                         m_stateIconPlacing->getSFSprite().getTexture()->getNativeHandle()),
            { 46.0f, 64.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f });
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Place units");
            ImGui::EndTooltip();
        }
    } else if (m_internalStateManager->getActiveStateE() == InternalState::Playback) {
        ImGui::Image(jt::SystemHelper::nativeHandleToImTextureId<ImTextureID>(
                         m_stateIconFighting->getSFSprite().getTexture()->getNativeHandle()),
            { 46.0f, 64.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f });
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Watch the battle evolve");
            ImGui::EndTooltip();
        }
    }
    ImGui::End();

    if (m_internalStateManager->getActiveStateE() == InternalState::Playback) {
        m_textTimeIndicator->draw(renderTarget());
    }

    if (m_internalStateManager->getActiveStateE() != InternalState::WaitForAllPlayers) {
        m_textRound->draw(renderTarget());
        m_textPlayerZeroHp->draw(renderTarget());
        m_textPlayerOneHp->draw(renderTarget());
    }
}

void StateGame::drawArrows() const
{
    for (auto const& a : m_simulationResultsForAllFrames.allFrames.at(m_tickId).m_arrows) {
        m_arrowShape->setPosition(a.currentPos);
        m_arrowShape->setShadow(jt::Color { 40, 40, 40, 255 },
            jt::Vector2f { 0.0f,
                -a.currentHeight
                    + m_terrain->getFieldHeight(a.currentPos) * terrainHeightScalingFactor });
        m_arrowShape->update(0.0f);
        m_arrowShape->setScale(jt::Vector2f { a.arrowScale, a.arrowScale });
        m_arrowShape->draw(renderTarget());
    }
}

void StateGame::updateServerInfo(nlohmann::json const& j) { m_terrain->from_json(j.at(jk::map)); }

void StateGame::endGame()
{
    if (m_hasEnded) {
        // trigger this function only once
        return;
    }
    m_hasEnded = true;
    m_running = false;

    getGame()->stateManager().switchState(std::make_shared<StateMenu>());
}

std::string StateGame::getName() const { return "State Game"; }

void StateGame::setConnection(
    std::shared_ptr<ClientNetworkConnection> connection, bool botAsPlayerZero, bool botAsPlayerOne)
{
    m_connection = connection;
    m_addBotAsPlayerZero = botAsPlayerZero;
    m_addBotAsPlayerOne = botAsPlayerOne;
}

void StateGame::resetAllUnits()
{
    auto propertiesForAllUnitsForInitialTick = m_simulationResultsForAllFrames.allFrames.front();
    auto propertiesForAllUnitsForFinalTick = m_simulationResultsForAllFrames.allFrames.back();
    if (propertiesForAllUnitsForInitialTick.m_units.size()
        != propertiesForAllUnitsForFinalTick.m_units.size()) {
        getGame()->logger().warning(
            "Unit size does not match for first and last tick of simulation", { "StateGame" });
    }
    for (auto idx = 0u; idx != propertiesForAllUnitsForInitialTick.m_units.size(); ++idx) {
        propertiesForAllUnitsForInitialTick.m_units[idx].unitAnim = "idle";
        propertiesForAllUnitsForInitialTick.m_units[idx].hpCurrent
            = propertiesForAllUnitsForInitialTick.m_units[idx].hpMax;
        propertiesForAllUnitsForInitialTick.m_units[idx].experience
            = propertiesForAllUnitsForFinalTick.m_units[idx].experience;
        propertiesForAllUnitsForInitialTick.m_units[idx].level
            = propertiesForAllUnitsForFinalTick.m_units[idx].level;
    }
    playbackOneFrame(propertiesForAllUnitsForInitialTick);
}

std::shared_ptr<InternalStateManager> StateGame::getStateManager()
{
    return m_internalStateManager;
}

std::shared_ptr<ServerConnection> StateGame::getServerConnection() { return m_serverConnection; }

std::shared_ptr<Terrain> StateGame::getTerrain() { return m_terrain; }

std::shared_ptr<TerrainRenderer> StateGame::getTerrainRenderer() { return m_terrainRenderer; }

std::shared_ptr<PlacementManager> StateGame::getPlacementManager() { return m_placementManager; }

std::shared_ptr<UnitInfoCollection> StateGame::getUnitInfo() { return m_unitInfo; }

int StateGame::getRound() { return m_round; }

std::shared_ptr<jt::ObjectGroup<Unit>> StateGame::getUnits() { return m_units; }

std::shared_ptr<jt::ObjectGroup<Critter>> StateGame::getCritters() { return m_critters; }

void StateGame::flashUnitsForUpgrade(const std::string& unitType)
{
    for (auto& u : *m_units) {
        auto unit = u.lock();
        if (!unit || unit->getPlayerID() != m_serverConnection->getPlayerId()) {
            continue;
        }
        if (unit->getInfo().type == unitType) {
            unit->flash();
        }
    }
    m_placementManager->flashForUpgrade(unitType);
}

std::shared_ptr<SelectUnitInfoCollection> StateGame::getStartingUnits() { return m_startingUnits; }
