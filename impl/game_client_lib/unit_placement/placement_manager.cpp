#include "placement_manager.hpp"
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <map/placement_area.hpp>
#include <map/terrain.hpp>
#include <math_helper.hpp>
#include <network_data/unit_client_to_server_data.hpp>
#include <network_data/unit_info_collection.hpp>
#include <object_group.hpp>
#include <unit_placement/placed_unit.hpp>
#include <unit_placement/placement_manager.hpp>
#include <unit_placement/upgrade_manager.hpp>
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

namespace {

auto const TextAlignedRight = [](auto const& s) {
    auto posX
        = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(s.c_str()).x);
    if (posX > ImGui::GetCursorPosX())
        ImGui::SetCursorPosX(posX);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().ItemSpacing.y / 2);
    ImGui::Text("%s", s.c_str());
};

} // namespace

PlacementManager::PlacementManager(std::shared_ptr<Terrain> world, int playerId,
    std::weak_ptr<PlayerIdDispatcher> playerIdDispatcher,
    std::shared_ptr<UnitInfoCollection> unitInfo)
    : m_world { world }
    , m_unitInfo { unitInfo }
{
    m_playerId = playerId;
    m_playerIdDispatcher = playerIdDispatcher;

    m_upgrades = std::make_shared<UpgradeManager>(unitInfo);
}

void PlacementManager::doCreate()
{
    auto playerIdDispatcher = m_playerIdDispatcher.lock();
    if (playerIdDispatcher == nullptr) {
        getGame()->logger().error("PlacementManager create called without valid PlayerIDDispatcher",
            { "PlacementManager" });
        return;
    }
    getGame()->logger().info("PlacementManager with playerId: " + std::to_string(m_playerId));
    m_blockedUnitPlacementAreas[AreaType::AREA_MAIN] = jt::dh::createShapeRect(
        getUnitPlacementArea(m_playerId == 1 ? 0 : 1, AreaType::AREA_MAIN),
        jt::Color { 20, 20, 20, 100 }, textureManager());
    m_blockedUnitPlacementAreas[AreaType::AREA_FLANK_TOP] = jt::dh::createShapeRect(
        getUnitPlacementArea(m_playerId == 1 ? 0 : 1, AreaType::AREA_FLANK_TOP),
        jt::Color { 20, 20, 20, 100 }, textureManager());
    m_blockedUnitPlacementAreas[AreaType::AREA_FLANK_BOT] = jt::dh::createShapeRect(
        getUnitPlacementArea(m_playerId == 1 ? 0 : 1, AreaType::AREA_FLANK_BOT),
        jt::Color { 20, 20, 20, 100 }, textureManager());
    m_placedUnits = std::make_shared<jt::ObjectGroup<PlacedUnit>>();

    m_sfxPlaceUnit = getGame()->audio().addTemporarySound("assets/sfx/drop_unit.wav");
    m_sfxPlaceUnit->setVolume(0.7f);
    m_sfxBuyUpgrade = getGame()->audio().addTemporarySound("assets/sfx/powerup.wav");
    m_sfxBuyUpgrade->setVolume(0.7f);

    m_fieldHighlight = jt::dh::createShapeRect(
        { 0, 0, terrainChunkSizeInPixel - 1, terrainChunkSizeInPixel - 1 },
        jt::Color { 0, 200, 0, 66 }, textureManager());
    m_fieldHighlight->setIgnoreCamMovement(true);
    m_fieldHighlight->setPosition({ -9999, -9999 });

    for (auto const& u : m_unitInfo->getTypes()) {
        m_imageUnits[u] = std::make_shared<jt::Sprite>(
            "assets/units/" + u + ".png", jt::Recti { 0, 0, 32, 32 }, textureManager());
        m_imageUnits[u]->setIgnoreCamMovement(true);
        m_imageUnits[u]->setColor(jt::Color { 50, 50, 50, 100 });
        m_imageUnits[u]->setPosition({ -9999, -9999 });
        if (m_playerId != 0) {
            m_imageUnits[u]->setScale({ -1, 1 });
        }
    }
}

void PlacementManager::doUpdate(float const elapsed)
{
    m_placedUnitsGO.update(elapsed);
    m_placedUnits->update(elapsed);
    m_tweens.update(elapsed);
    placeUnit();
    if (getGame()->input().mouse()->justPressed(jt::MouseButtonCode::MBRight)) {
        m_activeUnitType = "";
    }
    for (auto& area : m_blockedUnitPlacementAreas) {
        area->update(elapsed);
    }

    int posX, posY;
    auto fieldPos = m_world->getMappedFieldPosition(
        getGame()->input().mouse()->getMousePositionWorld(), posX, posY);
    if (canUnitBePlacedInField(fieldPos, posX, posY)) {
        m_fieldHighlight->setPosition({ fieldPos.x - terrainChunkSizeInPixelHalf,
            fieldPos.y - terrainChunkSizeInPixelHalf + 1 });
        if (!m_activeUnitType.empty()) {
            m_imageUnits[m_activeUnitType]->setOffset(
                { GP::UnitAnimationOffset().x + (m_playerId != 0 ? 32 : 0),
                    GP::UnitAnimationOffset().y
                        + m_world->getFieldHeight(fieldPos + terrainChunkSizeInPixelHalf - 1)
                            * -terrainHeightScalingFactor });
            m_imageUnits[m_activeUnitType]->setPosition({ fieldPos.x - terrainChunkSizeInPixelHalf,
                fieldPos.y - terrainChunkSizeInPixelHalf + 1 });
        }
    } else {
        m_fieldHighlight->setPosition({ -9999, -9999 });
        if (!m_activeUnitType.empty()) {
            m_imageUnits[m_activeUnitType]->setPosition({ -9999, -9999 });
        }
    }
    m_fieldHighlight->update(elapsed);
    if (!m_activeUnitType.empty()) {
        m_imageUnits[m_activeUnitType]->update(elapsed);
    }
}

void PlacementManager::doDraw() const
{
    if (!m_isActive) {
        return;
    }

    m_placedUnitsGO.draw();

    for (auto& area : m_blockedUnitPlacementAreas) {
        area->draw(renderTarget());
    }

    m_fieldHighlight->draw(renderTarget());
    if (!m_activeUnitType.empty()) {
        m_imageUnits.at(m_activeUnitType)->draw(renderTarget());
    }

    if (m_unitInfo) {
        auto const unlockedTypes = m_unlockedTypes;
        auto allTypes = m_unitInfo->getTypes();
        std::sort(allTypes.begin(), allTypes.end());
        std::vector<std::string> purchaseTypes;
        std::set_difference(allTypes.begin(), allTypes.end(), unlockedTypes.begin(),
            unlockedTypes.end(), std::back_inserter(purchaseTypes));

        constexpr auto buttonWidth = 106;

        ImGuiWindowFlags window_flags { ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
            | ImGuiWindowFlags_AlwaysAutoResize };
        ImGui::Begin("Unit placement", nullptr, window_flags);
        ImGui::Text("Gold: %i", m_availableFunds);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            ImGui::BeginTooltip();
            drawGoldStatistics();
            ImGui::EndTooltip();
        }
        ImGui::Separator();
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0, 0.5 });

        ImGui::Text("Hire");
        ImGui::BeginTable("UnitTable", 2);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 30);
        for (auto const& t : unlockedTypes) {
            auto const& u = m_unitInfo->getInfoForType(t);
            bool const canBuy = (m_availableFunds >= u.cost);

            ImGui::TableNextColumn();
            TextAlignedRight(std::to_string(u.cost));

            ImGui::TableNextColumn();
            ImGui::BeginDisabled(!canBuy);
            if (m_activeUnitType == u.type) {
                auto const colorSelected = IM_COL32(0, 120, 00, 255);
                ImGui::PushStyleColor(ImGuiCol_Button, colorSelected);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorSelected);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorSelected);
                ImGui::Button(u.type.c_str(), { buttonWidth, 0 });
                ImGui::PopStyleColor(3);
            } else {
                if (ImGui::Button(u.type.c_str(), { buttonWidth, 0 })) {
                    getGame()->logger().debug("select: " + u.type);
                    m_activeUnitType = u.type;
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                    ImGui::SetTooltip("%s", u.description.c_str());
                }
            }
            ImGui::EndDisabled();
        }
        ImGui::BeginDisabled(m_round < 2);
        if (ImGui::Button("+")) {
            m_showUnlockUnitWindow = true;
        }
        ImGui::EndDisabled();

        ImGui::EndTable();

        ImGui::PopStyleVar();

        ImGui::Separator();

        bool const canTakeCredit = m_creditDebt == 0;
        ImGui::BeginDisabled(!canTakeCredit);
        if (ImGui::Button("Take credit (+100 now, -150 next round)")) {
            m_availableFunds += 100;
            m_creditDebt = 150;
        }
        ImGui::EndDisabled();

        ImGui::Separator();

        bool const canUndoLastBuy = !m_placedUnits->empty();
        ImGui::BeginDisabled(!canUndoLastBuy);
        if (ImGui::Button("Undo last purchase")) {
            auto const& unit = m_placedUnits->back().lock();
            if (!unit) {
                throw std::logic_error { "invalid unit in placed Units list" };
            }

            int posX { 0 };
            int posY { 0 };
            (void)m_world->getMappedFieldPosition(unit->getPosition(), posX, posY);

            fieldInUse(posX, posY) = false;

            sellUnit(unit->getInfo());

            m_placedUnits->pop_back();
            m_placedUnitsGO.pop_back();
        }
        ImGui::EndDisabled();
        ImGui::End();

        if (m_showUnlockUnitWindow) {
            ImGuiWindowFlags window_flags { ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
                | ImGuiWindowFlags_AlwaysAutoResize };
            ImGui::Begin("Unlock Units", nullptr, window_flags);
            ImGui::Text("Gold: %i", m_availableFunds);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::BeginTooltip();
                drawGoldStatistics();
                ImGui::EndTooltip();
            }
            ImGui::Text("Available Unit Unlocks: %i / %i",
                m_unitUnlocksAvailable - m_unitsUnlockedThisRound, m_unitUnlocksAvailable);
            ImGui::Separator();
            if (!purchaseTypes.empty()) {
                ImGui::Text("Unlock");
                ImGui::BeginTable("UnitTable", 2);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 30);
                for (auto const& t : purchaseTypes) {
                    auto const& u = m_unitInfo->getInfoForType(t);
                    bool const canUnlock = (m_availableFunds >= u.unlockCost)
                        && (m_unitsUnlockedThisRound < m_unitUnlocksAvailable);

                    ImGui::TableNextColumn();
                    TextAlignedRight(std::to_string(u.unlockCost));

                    ImGui::TableNextColumn();
                    ImGui::BeginDisabled(!canUnlock);
                    if (ImGui::Button(u.type.c_str(), { buttonWidth, 0 })) {
                        unlockType(t);
                        m_availableFunds -= u.unlockCost;
                        m_showUnlockUnitWindow = false;
                    }
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                        std::string const str = u.description + " Cost: " + std::to_string(u.cost);
                        ImGui::SetTooltip("%s", str.c_str());
                    }
                    ImGui::EndDisabled();
                }
                ImGui::EndTable();
            } else {
                ImGui::Text("%s", "No more Units to unlock");
            }
            if (ImGui::Button("close")) {
                m_showUnlockUnitWindow = false;
            }
            ImGui::End();
        }
    }
}

void PlacementManager::placeUnit()
{
    if (m_activeUnitType == "") {
        return;
    }

    if (m_availableFunds < m_unitInfo->getInfoForType(m_activeUnitType).cost) {
        m_activeUnitType = "";
        return;
    }

    if (getGame()->input().mouse()->justPressed(jt::MouseButtonCode::MBLeft)
        && getGame()->gfx().window().shouldProcessMouse()) {
        auto playerIdDispatcher = m_playerIdDispatcher.lock();
        if (playerIdDispatcher == nullptr) {
            getGame()->logger().warning(
                "place unit with invalid playerIDDispatcher called", { "PlacementManager" });
            return;
        }

        int posX, posY;
        auto fieldPos = m_world->getMappedFieldPosition(
            getGame()->input().mouse()->getMousePositionWorld(), posX, posY);
        if (!canUnitBePlacedInField(fieldPos, posX, posY)) {
            getGame()->logger().info(
                "tried to place unit in invalid position", { "PlacementManager" });
            return;
        }

        auto const info = m_unitInfo->getInfoForType(m_activeUnitType);
        auto unit = std::make_shared<PlacedUnit>(info);
        m_placedUnits->push_back(unit);
        m_placedUnitsGO.add(unit);
        fieldInUse(posX, posY) = true;
        unit->setGameInstance(getGame());
        unit->create();

        unit->setOffset({ 0,
            m_world->getFieldHeight(fieldPos + terrainChunkSizeInPixelHalf - 1)
                * -terrainHeightScalingFactor });
        fieldPos -= terrainChunkSizeInPixelHalf; // offset position to top left corner of field
        unit->setPosition(fieldPos);

        unit->setIDs(m_unitIdManager.getIdForPlayer(m_playerId), m_playerId);

        auto tw = unit->createInitialTween();
        tw->addCompleteCallback([this]() { m_sfxPlaceUnit->play(); });
        m_tweens.add(tw);

        m_availableFunds -= info.cost;

        buyUnit(info.type);
        if (!getGame()->input().keyboard()->pressed(jt::KeyCode::LShift)) {
            m_activeUnitType = "";
        }
    }
}

bool PlacementManager::canUnitBePlacedInField(jt::Vector2f const& pos, int const x, int const y)
{
    static std::vector<AreaType> areas { AreaType::AREA_MAIN, AreaType::AREA_FLANK_TOP,
        AreaType::AREA_FLANK_BOT };
    bool inValidArea { false };
    for (auto& area : areas) {
        if (jt::MathHelper::checkIsIn(getUnitPlacementArea(m_playerId, area), pos)) {
            inValidArea = true;
            break;
        }
    }
    return inValidArea && !fieldInUse(x, y);
}

bool& PlacementManager::fieldInUse(int const x, int const y) const
{
    return m_placedUnitsMap[x + y * terrainWidthInChunks];
}

std::shared_ptr<UnitInfoCollection> PlacementManager::getUnitInfoCollection() const
{
    return m_unitInfo;
}

std::vector<UnitClientToServerData> PlacementManager::getPlacedUnitDataForRoundStart() const
{
    std::vector<UnitClientToServerData> properties;
    for (auto const& u : *m_placedUnits) {
        properties.push_back(u.lock()->saveState());
    }
    return properties;
}

std::shared_ptr<jt::ObjectGroup<PlacedUnit>> const& PlacementManager::getPlacedUnits() const
{
    return m_placedUnits;
}

void PlacementManager::clearPlacedUnits()
{
    m_placedUnitsGO.clear();
    m_placedUnits->clear();
}

void PlacementManager::setActive(bool active)
{
    m_isActive = active;
    if (active) {
        m_unitsUnlockedThisRound = 0;
    }
}

void PlacementManager::setRound(int round) { m_round = round; }

void PlacementManager::addFunds(int funds)
{
    m_availableFunds += funds - m_creditDebt;
    m_fundsGainedInLastRound = funds;
}

void PlacementManager::addLoserBonus(int bonus)
{
    m_availableFunds += bonus;
    m_looserBonusFromLastRound = bonus;
}

int PlacementManager::getFunds() const { return m_availableFunds; }

void PlacementManager::spendFunds(int expenses) { m_availableFunds -= expenses; }

void PlacementManager::recordFundsFromLastRound()
{
    m_fundsFromLastRound = m_availableFunds;
    m_looserBonusFromLastRound = 0;
}

void PlacementManager::unlockType(std::string const& type) const
{
    m_unlockedTypes.insert(type);
    ++m_unitsUnlockedThisRound;
}

void PlacementManager::flashForUpgrade(std::string const& unitType)
{
    for (auto& u : *m_placedUnits) {
        auto unit = u.lock();
        if (unit && unit->getInfo().type == unitType) {
            unit->flash();
        }
    }
}

void PlacementManager::buyUnit(std::string const& type)
{
    if (m_boughtUnits.count(type) == 0) {
        m_boughtUnits[type] = 0;
    }
    m_boughtUnits.at(type)++;

    if (m_boughtUnits.at(type) >= 3) {
        m_boughtUnits.at(type) -= 3;
        m_unitInfo->multiplyPriceForUnitBy(type, 1.08f);
    }
}

void PlacementManager::sellUnit(UnitInfo const& unitInfo) const
{
    m_availableFunds += unitInfo.cost;

    auto const& type = unitInfo.type;
    if (m_boughtUnits.count(type) == 0) {
        m_boughtUnits[type] = 0;
        getGame()->logger().warning("Selling unit that was not bought before");
    }
    m_boughtUnits.at(type)--;

    if (m_boughtUnits.at(type) <= -1) {
        m_boughtUnits.at(type) += 3;
        m_unitInfo->multiplyPriceForUnitBy(type, 1.0f / 1.08f);
    }
}

void PlacementManager::sellUnitForPreviousRound(UnitInfo const& unitInfo) const
{
    m_availableFunds += unitInfo.cost;
}

std::shared_ptr<UpgradeManager> PlacementManager::upgrades() const { return m_upgrades; }

void PlacementManager::resetCreditDebt()
{
    m_creditDebtFromLastRound = m_creditDebt;
    m_creditDebt = 0;
}

std::string PlacementManager::getActiveUnitType() const { return m_activeUnitType; }

void PlacementManager::resetActiveUnitType() const { m_activeUnitType.clear(); }

void PlacementManager::drawGoldStatistics() const
{
    ImGui::Text("Income Statistics");
    if (ImGui::BeginTable("goldStatistics", 2)) {

        ImGui::TableNextColumn();
        ImGui::Text("%i", m_fundsFromLastRound);
        ImGui::TableNextColumn();
        ImGui::Text("last round");

        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        ImGui::Text("%i", m_fundsGainedInLastRound);
        ImGui::PopStyleColor();
        ImGui::TableNextColumn();
        ImGui::Text("income");

        if (m_creditDebtFromLastRound != 0) {
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
            ImGui::Text("-%i", m_creditDebtFromLastRound);
            ImGui::PopStyleColor();
            ImGui::TableNextColumn();
            ImGui::Text("debt");
        }
        if (m_looserBonusFromLastRound != 0) {
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
            ImGui::Text("%i", m_looserBonusFromLastRound);
            ImGui::PopStyleColor();
            ImGui::TableNextColumn();
            ImGui::Text("looser bonus");
        }
        ImGui::EndTable();
    }
    ImGui::Text("Gold: %i", m_availableFunds);
}
