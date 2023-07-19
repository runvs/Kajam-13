#include "upgrade_manager.hpp"
#include "upgrade_info_client.hpp"

UpgradeManager::UpgradeManager(std::shared_ptr<UnitInfoCollection> unitInfo)
    : m_unitInfoCollection { unitInfo }
{
    // Build list of possible upgrades
    for (auto const& u : m_unitInfoCollection->getUnits()) {
        for (auto const& upg : u.possibleUpgrades) {

            UpgradeInfoClient upgradeInfoClient;
            upgradeInfoClient.info = upg;
            upgradeInfoClient.icon = nullptr;
            m_possibleUpgrades[0][u.type].emplace_back(upgradeInfoClient);
            m_possibleUpgrades[1][u.type].emplace_back(upgradeInfoClient);
        }
    }
}

void UpgradeManager::buyUpgrade(
    int playerID, std::string const& unitType, std::string const& upgrade) const
{
    auto& vec = m_possibleUpgrades[playerID].at(unitType);
    UpgradeInfoClient upgradeInfoClient;
    upgradeInfoClient.info = m_unitInfoCollection->getUpgradeForUnit(unitType, upgrade);
    m_boughtUpgrades[playerID][unitType].push_back(upgradeInfoClient);

    std::erase_if(vec, [&](auto const& v) { return v.info.name == upgrade; });
}

std::vector<UpgradeInfoClient>& UpgradeManager::getPossibleUpgradesForUnit(
    int playerID, std::string const& unitType) const
{
    return m_possibleUpgrades[playerID][unitType];
}

std::vector<UpgradeInfoClient>& UpgradeManager::getBoughtUpgradesForUnit(
    int playerID, std::string const& unitType) const
{
    return m_boughtUpgrades[playerID][unitType];
}
