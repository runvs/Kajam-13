#ifndef JAMTEMPLATE_UPGRADE_MANAGER_HPP
#define JAMTEMPLATE_UPGRADE_MANAGER_HPP

#include <network_data/unit_info_collection.hpp>
#include <upgrade_info_client.hpp>
#include <map>
#include <string>

class UpgradeManager {
public:
    explicit UpgradeManager(std::shared_ptr<UnitInfoCollection> unitInfo);

    void buyUpgrade(int playerID, std::string const& unitType, const std::string& upgrade) const;
    std::vector<UpgradeInfoClient>& getPossibleUpgradesForUnit(
        int playerID, std::string const& unitType) const;
    std::vector<UpgradeInfoClient>& getBoughtUpgradesForUnit(
        int playerID, std::string const& unitType) const;

private:
    std::shared_ptr<UnitInfoCollection> m_unitInfoCollection;

    mutable std::map<int, std::map<std::string, std::vector<UpgradeInfoClient>>> m_possibleUpgrades;
    mutable std::map<int, std::map<std::string, std::vector<UpgradeInfoClient>>> m_boughtUpgrades;
};

#endif // JAMTEMPLATE_UPGRADE_MANAGER_HPP
