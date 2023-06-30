#include "placement_manager.hpp"
#include <network_data/unit_info_collection.hpp>
#include "upgrade_manager.hpp"
#include <drawable_helpers.hpp>
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <map/placement_area.hpp>
#include <map/terrain.hpp>
#include <math_helper.hpp>
#include <network_data/unit_client_to_server_data.hpp>
#include <object_group.hpp>
#include <unit_placement/placed_unit.hpp>
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

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
}

void PlacementManager::doUpdate(const float elapsed)
{
    m_placedUnitsGO.update(elapsed);
    m_placedUnits->update(elapsed);
    m_tweens.update(elapsed);
    placeUnit();
    for (auto& area : m_blockedUnitPlacementAreas) {
        area->update(elapsed);
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

    if (m_unitInfo) {
        ImGui::Begin("unit placement");

        ImGui::Text("Gold: %i", m_availableFunds);

        ImGui::Separator();
        ImGui::Text("Unlock");
        auto unlockedTypes = m_unlockedTypes;
        auto allTypes = m_unitInfo->getTypes();
        std::sort(unlockedTypes.begin(), unlockedTypes.end());
        std::sort(allTypes.begin(), allTypes.end());
        std::vector<std::string> purchaseTypes;
        std::set_difference(allTypes.begin(), allTypes.end(), unlockedTypes.begin(),
            unlockedTypes.end(), std::back_inserter(purchaseTypes));

        for (auto const& t : purchaseTypes) {
            auto const& u = m_unitInfo->getInfoForType(t);
            bool const canUnlock = (m_availableFunds >= u.unlockCost);
            ImGui::BeginDisabled(!canUnlock);
            std::string const buttonString = u.type + " (" + std::to_string(u.unlockCost) + ")";
            if (ImGui::Button(buttonString.c_str())) {
                unlockType(t);
                m_availableFunds -= u.unlockCost;
            }
            ImGui::EndDisabled();
        }

        ImGui::Separator();
        ImGui::Text("Hire");
        for (auto const& t : unlockedTypes) {
            auto const& u = m_unitInfo->getInfoForType(t);
            std::string const buttonString = u.type + " (" + std::to_string(u.cost) + ")";
            bool const canBuy = (m_availableFunds >= u.cost);
            ImGui::BeginDisabled(!canBuy);

            if (ImGui::Button(buttonString.c_str())) {
                getGame()->logger().debug("select: " + u.type);
                m_activeUnitType = u.type;
            }

            ImGui::EndDisabled();
        }
        ImGui::End();
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

    if (getGame()->input().mouse()->justPressed(jt::MouseButtonCode::MBRight)) {
        auto playerIdDispatcher = m_playerIdDispatcher.lock();
        if (playerIdDispatcher == nullptr) {
            getGame()->logger().warning(
                "place unit with invalid playerIDDispatcher called", { "PlacementManager" });
            return;
        }

        int posX, posY;
        auto fieldPos = m_world->getMappedFieldPosition(
            getGame()->input().mouse()->getMousePositionWorld(), posX, posY);
        static std::vector<AreaType> areas { AreaType::AREA_MAIN, AreaType::AREA_FLANK_TOP,
            AreaType::AREA_FLANK_BOT };
        bool inValidArea { false };
        for (auto& area : areas) {
            if (jt::MathHelper::checkIsIn(getUnitPlacementArea(m_playerId, area), fieldPos)) {
                inValidArea = true;
                break;
            }
        }
        if (!inValidArea || fieldInUse(posX, posY)) {
            // TODO Show some visual representation or play a sound that placing a unit here is not
            getGame()->logger().info(
                "tried to place unit in invalid position", { "PlacementManager" });
            // possible.
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
            m_world->getFieldHeight(fieldPos + terrainChunkSizeInPixelHalf)
                * -terrainHeightScalingFactor });
        fieldPos -= terrainChunkSizeInPixelHalf; // offset position to top left corner of field
        unit->setPosition(fieldPos);

        unit->setIDs(m_unitIdManager.getIdForPlayer(m_playerId), m_playerId);

        auto tw = unit->createInitialTween();
        tw->addCompleteCallback([this]() { m_sfxPlaceUnit->play(); });
        m_tweens.add(tw);

        m_availableFunds -= info.cost;

        buyUnit(info.type);
    }
}

bool& PlacementManager::fieldInUse(int const x, int const y)
{
    return m_placedUnitsMap[x + y * terrainWidthInChunks];
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

void PlacementManager::clearPlacedUnits() { m_placedUnitsGO.clear(); }

void PlacementManager::setActive(bool active) { m_isActive = active; }

void PlacementManager::addFunds(int funds) { m_availableFunds += funds; }

int PlacementManager::getFunds() const { return m_availableFunds; }

void PlacementManager::unlockType(const std::string& type) const
{
    // TODO check for duplicates
    m_unlockedTypes.push_back(type);
}

// void PlacementManager::buyUpgrade(std::string const& unitType, const std::string& upgrade) const
//{
//     m_sfxBuyUpgrade->play();
// }

// std::vector<UpgradeInfo>& PlacementManager::getBoughtUpgradesForUnit(
//     const std::string& unitType) const
//{
//     return m_boughtUpgradesPlayer0[unitType];
// }

void PlacementManager::flashForUpgrade(std::string const& unitType)
{
    for (auto& u : *m_placedUnits) {
        auto unit = u.lock();
        if (unit && unit->getInfo().type == unitType) {
            unit->flash();
        }
    }
}
void PlacementManager::buyUnit(const std::string& type)
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
std::shared_ptr<UpgradeManager> PlacementManager::upgrades() { return m_upgrades; }
std::shared_ptr<UpgradeManager> PlacementManager::upgrades() const { return m_upgrades; }
