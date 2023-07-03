#include "upgrade_info_client.hpp"

std::vector<UpgradeInfo> convertUpgradeInfoClientVectorToUpgradeInfoVector(
    std::vector<UpgradeInfoClient> const& upgradeInfoClients)
{
    std::vector<UpgradeInfo> upgradeInfos;
    upgradeInfos.resize(upgradeInfoClients.size());
    for (auto i = 0u; i != upgradeInfoClients.size(); ++i) {
        upgradeInfos[i] = upgradeInfoClients[i].info;
    }
    return upgradeInfos;
}
