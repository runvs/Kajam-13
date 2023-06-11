#include "unit_info_collection.hpp"
#include <json_keys.hpp>
#include <nlohmann.hpp>
#include <fstream>
#include <stdexcept>
#include <string>

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
    throw std::invalid_argument { "unit info for type " + type + " not found" };
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
void UnitInfoCollection::unlockType(const std::string& type)
{
    if (std::count(m_types.begin(), m_types.end(), type) == 0) {
        throw std::invalid_argument { "unit info for type " + type + " not found" };
    }
    if (std::count(m_unlockedTypes.begin(), m_unlockedTypes.end(), type) == 1) {
        m_logger.warning("type " + type + " already unlocked");
    }
    m_unlockedTypes.push_back(type);
}
std::vector<std::string> UnitInfoCollection::getUnlockedTypes() const { return m_unlockedTypes; }

void to_json(nlohmann::json& j, UnitInfoCollection& p)
{
    j = nlohmann::json { { jk::units, p.getUnits() } };
}
void from_json(const nlohmann::json& j, UnitInfoCollection& p)
{
    j.at(jk::units).get_to(p.getUnits());
}
