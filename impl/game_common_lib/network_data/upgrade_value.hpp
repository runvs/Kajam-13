#ifndef JAMTEMPLATE_UPGRADE_VALUE_HPP
#define JAMTEMPLATE_UPGRADE_VALUE_HPP

#include <nlohmann.hpp>

struct UpgradeValue {
    float factor { 0.0f };
    float add { 0.0f };
};

bool isDefault(UpgradeValue const& upg);

void to_json(nlohmann::json& j, const UpgradeValue& p);
void from_json(const nlohmann::json& j, UpgradeValue& p);

#endif // JAMTEMPLATE_UPGRADE_VALUE_HPP
