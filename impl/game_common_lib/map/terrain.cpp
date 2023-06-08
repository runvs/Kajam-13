#include "vector.hpp"
#include <map/terrain.hpp>
#include <math_helper.hpp>
#include <nlohmann.hpp>
#include <cmath>
#include <fstream>
#include <iostream>

namespace {

struct TowerInfo {
    int playerId {};
    jt::Vector2f position {};
};

void to_json(nlohmann::json& j, TowerInfo const& v)
{
    j = nlohmann::json { { "player", v.playerId }, { "x", v.position.x }, { "y", v.position.y } };
}

void from_json(nlohmann::json const& j, TowerInfo& v)
{
    j.at("player").get_to(v.playerId);
    j.at("x").get_to(v.position.x);
    j.at("y").get_to(v.position.y);
}

struct Map {
    float heights[terrainWidthInChunks * terrainHeightInChunks];
    std::vector<TowerInfo> towers;
};

void to_json(nlohmann::json& j, Map const& v)
{
    j = nlohmann::json { { "heights", v.heights }, { "towers", v.towers } };
}

void from_json(nlohmann::json const& j, Map& v)
{
    j.at("heights").get_to(v.heights);
    j.at("towers").get_to(v.towers);
}

std::size_t posToIndex(int x, int y) { return x + y * terrainWidthInChunks; }

} // namespace

Terrain::Terrain(std::string const& mapFilename)
{
    // load map from file
    parseMapFromFilename(mapFilename);
}

float Terrain::getChunkHeight(int x, int y) const
{
    if ((x < 0) || (x >= terrainWidthInChunks) || (y < 0) || (y >= terrainHeightInChunks)) {
        return 0.0f;
    }
    auto const& chunk = m_chunks[y * terrainWidthInChunks + x];
    return chunk.height;
}

float Terrain::getSlopeAt(jt::Vector2f const& pos, jt::Vector2f const& dir) const
{
    // TODO limit dir to +/-1 values
    auto roundedDir = jt::Vector2f { std::round(dir.x), std::round(dir.y) };

    const auto x = static_cast<int>(pos.x / terrainChunkSizeInPixel);
    const auto y = static_cast<int>(pos.y / terrainChunkSizeInPixel);
    if ((x < 0) || (x >= terrainWidthInChunks) || (y < 0) || (y > terrainHeightInChunks)) {
        return 0.0f;
    }
    auto const centerHeight = getChunkHeight(x, y);

    const auto ox = x + roundedDir.x;
    const auto oy = y + roundedDir.y;
    auto heightInDirection = getChunkHeight(ox, oy);
    auto diff = (heightInDirection - centerHeight);
    if (heightInDirection < centerHeight) {
        return 0;
    }
    // check backwards dir
    if (heightInDirection == centerHeight) {
        const auto ox2 = x - roundedDir.x;
        const auto oy2 = y - roundedDir.y;
        heightInDirection = getChunkHeight(ox2, oy2);
        diff = -(heightInDirection - centerHeight);
        if (heightInDirection < centerHeight) {
            return 0;
        }
    }

    return diff;
}

jt::Vector2f Terrain::getMappedFieldPosition(jt::Vector2f const& pos) const
{
    return { pos.x - static_cast<int>(pos.x) % terrainChunkSizeInPixel
            + terrainChunkSizeInPixel / 2,
        pos.y - static_cast<int>(pos.y) % terrainChunkSizeInPixel + terrainChunkSizeInPixel / 2 };
}

void Terrain::parseMapFromFilename(std::string const& fileName)
{
    std::ifstream fileStream { fileName };
    auto const map = nlohmann::json::parse(fileStream).get<Map>();
    for (unsigned short y { 0 }; y < terrainHeightInChunks; ++y) {
        for (unsigned short x { 0 }; x < terrainWidthInChunks; ++x) {
            m_chunks[posToIndex(x, y)] = { x, y, map.heights[posToIndex(x, y)] };
        }
    }
}
