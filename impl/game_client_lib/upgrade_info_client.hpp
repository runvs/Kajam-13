#ifndef JAMTEMPLATE_UPGRADE_INFO_CLIENT_HPP
#define JAMTEMPLATE_UPGRADE_INFO_CLIENT_HPP

#include <network_data/upgrade_info.hpp>
#include <sprite.hpp>
#include <memory>

struct UpgradeInfoClient {
    UpgradeInfo info;
    std::shared_ptr<jt::Sprite> icon { nullptr };
};

std::vector<UpgradeInfo> convertUpgradeInfoClientVectorToUpgradeInfoVector(
    std::vector<UpgradeInfoClient> const& upgradeInfoClients);

#endif // JAMTEMPLATE_UPGRADE_INFO_CLIENT_HPP
