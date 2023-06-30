#ifndef JAMTEMPLATE_UNIT_INFO_COLLECTION_HPP
#define JAMTEMPLATE_UNIT_INFO_COLLECTION_HPP

#include "log/logger_interface.hpp"
#include "unit_info.hpp"
#include <nlohmann.hpp>
#include <string>
#include <vector>

class UnitInfoCollection {
public:
    UnitInfoCollection(jt::LoggerInterface& logger,
        std::string const& infoFilename = "assets/units/all_units.json");
    UnitInfoCollection(jt::LoggerInterface& logger, std::vector<UnitInfo> infos);
    UnitInfo const& getInfoForType(std::string const& type) const;
    void multiplyPriceForUnitBy(const std::string& type, float factor);
    std::vector<std::string> getTypes() const;

    std::vector<UnitInfo>& getUnits();
    std::vector<UnitInfo> const& getUnits() const;

    UpgradeInfo const& getUpgradeForUnit(
        std::string const& unitType, std::string const& upgradeName) const;

private:
    jt::LoggerInterface& m_logger;
    std::vector<UnitInfo> m_infos;
    // cached vector of types
    std::vector<std::string> m_types;

    std::vector<std::string> m_unlockedTypes;

    void parseUnitInfosFromFilename(std::string const& fileName);
    void createTypesVector();
};

void to_json(nlohmann::json& j, UnitInfoCollection& p);
void from_json(const nlohmann::json& j, UnitInfoCollection& p);

#endif // JAMTEMPLATE_UNIT_INFO_COLLECTION_HPP
