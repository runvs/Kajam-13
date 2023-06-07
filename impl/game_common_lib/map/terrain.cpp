#include <map/terrain.hpp>
#include <math_helper.hpp>
#include <nlohmann.hpp>
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

void printSlopeAt(Terrain const& terrain, int x, int y, jt::Vector2f const& dir)
{
    auto const slope = terrain.getSlopeAt(
        jt::Vector2f { x * terrainChunkSizeInPixel, y * terrainChunkSizeInPixel }, dir);
    std::cerr << "Slope at [" << x << "," << y << "] dir[" << dir.x << "," << dir.y
              << "]: " << slope << std::endl;
}

} // namespace

Terrain::Terrain()
{
    // load map from file
    parseMapFromFilename("assets/maps/map_de_dust_2.json");

    // test slopes
    // TODO replace by unit tests
    printSlopeAt(*this, 1, 1, jt::Vector2f { 1.0f, 0.0f });
    printSlopeAt(*this, 1, 1, jt::Vector2f { -1.0f, 0.0f });
    printSlopeAt(*this, 1, 1, jt::Vector2f { 0.0f, 1.0f });
    printSlopeAt(*this, 1, 1, jt::Vector2f { 0.0f, -1.0f });
    printSlopeAt(*this, 7, 8, jt::Vector2f { 1.0f, 0.0f });
}

float Terrain::getChunkHeight(jt::Vector2f const& pos) const
{
    int x = static_cast<int>(pos.x / terrainChunkSizeInPixel);
    int y = static_cast<int>(pos.y / terrainChunkSizeInPixel);
    if ((x < 0) || (x > terrainWidthInChunks) || (y < 0) || (y > terrainHeightInChunks)) {
        return 0.0f;
    }
    auto const& chunk = m_chunks[y * terrainWidthInChunks + x];
    return chunk.height;
}

float Terrain::getSlopeAt(jt::Vector2f const& pos, jt::Vector2f const& dir) const
{
    auto const heightPos = getChunkHeight(pos);

    // determine neighbouring chunk heights
    float heightPosInX { 0.0f };
    float heightPosInY { 0.0f };
    if (dir.x > 0) {
        // right chunk
        heightPosInX = getChunkHeight(pos + jt::Vector2f { terrainChunkSizeInPixel, 0.0f });
    } else if (dir.x < 0) {
        // left chunk
        heightPosInX = getChunkHeight(pos + jt::Vector2f { -terrainChunkSizeInPixel, 0.0f });
    }
    if (dir.y > 0) {
        // bot chunk
        heightPosInY = getChunkHeight(pos + jt::Vector2f { 0.0f, terrainChunkSizeInPixel });
    } else if (dir.y < 0) {
        // top chunk
        heightPosInY = getChunkHeight(pos + jt::Vector2f { 0.0f, -terrainChunkSizeInPixel });
    }

    return (dir.x * (heightPos - heightPosInX) + dir.y * (heightPos - heightPosInY)) / 2.0f;
}

void Terrain::parseMapFromFilename(std::string const& fileName)
{
    std::ifstream fileStream { fileName };
    auto const map = nlohmann::json::parse(fileStream).get<Map>();
    for (unsigned short h { 0 }; h < terrainHeightInChunks; ++h) {
        auto const wOffset = h * terrainWidthInChunks;
        for (unsigned short w { 0 }; w < terrainWidthInChunks; ++w) {
            m_chunks[wOffset + w] = { w, h, map.heights[wOffset + w] };
        }
    }
}
