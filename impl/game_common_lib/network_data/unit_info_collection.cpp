#include "unit_info_collection.hpp"
#include "json_keys.hpp"
#include <fstream>
#include <stdexcept>

UnitInfoCollection::UnitInfoCollection(jt::LoggerInterface& logger, const std::string& infoFilename)
    : m_logger { logger }
{
    parseUnitInfosFromFilename(infoFilename);

    createTypesVector();
}

UnitInfoCollection::UnitInfoCollection(jt::LoggerInterface& logger, std::vector<UnitInfo> infos)
    : m_logger { logger }
{
    m_infos = infos;
    createTypesVector();
}

void UnitInfoCollection::createTypesVector()
{
    for (auto const& i : m_infos) {
        m_types.push_back(i.type);
    }
}

UnitInfo const& UnitInfoCollection::getInfoForType(const std::string& type) const
{
    for (auto const& i : m_infos) {
        if (i.type == type) {
            return i;
        }
    }
    throw std::invalid_argument { "unit info for type '" + type + "' not found" };
}

std::vector<std::string> UnitInfoCollection::getTypes() const { return m_types; }

std::vector<UnitInfo>& UnitInfoCollection::getUnits() { return m_infos; }

std::vector<UnitInfo> const& UnitInfoCollection::getUnits() const { return m_infos; }
void UnitInfoCollection::parseUnitInfosFromFilename(const std::string& fileName)
{
    std::vector<std::string> fileNames;
    {
        std::ifstream infile { fileName };
        nlohmann::json j = nlohmann::json::parse(infile);
        j.at(jk::files).get_to(fileNames);
    }
    for (auto const& fileName : fileNames) {
        m_logger.info("Parse unit json file: " + fileName, { "UnitInfoCollection" });
        std::ifstream unitFile { fileName };
        nlohmann::json j = nlohmann::json::parse(unitFile);
        m_infos.emplace_back(j);
    }

    m_logger.info(
        "Parsed a total of " + std::to_string(m_infos.size()) + " units", { "UnitInfoCollection" });
}

UpgradeInfo const& UnitInfoCollection::getUpgradeForUnit(
    std::string const& unitType, std::string const& upgradeName) const
{
    for (auto const& u : m_infos) {
        if (u.type != unitType) {
            continue;
        }
        for (auto const& upg : u.possibleUpgrades) {
            if (upg.name == upgradeName) {
                return upg;
            }
        }
    }
    throw std::invalid_argument { "No upgrade " + upgradeName + " for unit: " + unitType };
}
void UnitInfoCollection::multiplyPriceForUnitBy(const std::string& type, float factor)
{
    for (auto& u : m_infos) {
        if (u.type == type) {
            u.cost *= factor;
        }
    }
}

void to_json(nlohmann::json& j, UnitInfoCollection& p)
{
    j = nlohmann::json { { jk::units, p.getUnits() } };
}
void from_json(const nlohmann::json& j, UnitInfoCollection& p)
{
    j.at(jk::units).get_to(p.getUnits());
}
