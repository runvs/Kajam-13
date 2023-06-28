#include "upgrade_manager.hpp"
UpgradeManager::UpgradeManager(std::shared_ptr<UnitInfoCollection> unitInfo)
    : m_unitInfoCollection { unitInfo }
{

    // Build list of possible upgrades
    for (auto const& u : m_unitInfoCollection->getUnits()) {
        for (auto const& upg : u.possibleUpgrades) {
            m_possibleUpgrades[0][u.type].push_back(upg);
            m_possibleUpgrades[1][u.type].push_back(upg);
        }
    }
}
void UpgradeManager::buyUpgrade(
    int playerID, std::string const& unitType, std::string const& upgrade) const
{
    auto& vec = m_possibleUpgrades[playerID].at(unitType);
    auto const& upg = m_unitInfoCollection->getUpgradeForUnit(unitType, upgrade);
    m_boughtUpgrades[playerID][unitType].push_back(upg);
    std::erase_if(vec, [&](auto const& v) { return v.name == upgrade; });
    //    m_sfxBuyUpgrade->play();
}

std::vector<UpgradeInfo>& UpgradeManager::getPossibleUpgradesForUnit(
    int playerID, std::string const& unitType) const
{
    return m_possibleUpgrades[playerID].at(unitType);
}

std::vector<UpgradeInfo>& UpgradeManager::getBoughtUpgradesForUnit(
    int playerID, std::string const& unitType) const
{
    return m_boughtUpgrades[playerID][unitType];
}
