#ifndef JAMTEMPLATE_TERRAIN_HPP
#define JAMTEMPLATE_TERRAIN_HPP

#include <vector.hpp>
#include <array>
#include <string>

constexpr int terrainWidthPerPlayerInChunks { 16 };
constexpr int terrainHeightInChunksPerPlayerInChunks { 19 };
constexpr int terrainFlankHeightInChunks { 3 };
constexpr int terrainWidthInChunks { terrainWidthPerPlayerInChunks * 2 };
constexpr int terrainHeightInChunks { terrainHeightInChunksPerPlayerInChunks
    + terrainFlankHeightInChunks * 2 };
constexpr int terrainChunkSizeInPixel { 16 };
constexpr int terrainChunkSizeInPixelHalf { terrainChunkSizeInPixel / 2 };
constexpr int const terrainFlankHeightInPixel { terrainFlankHeightInChunks
    * terrainChunkSizeInPixel };
constexpr float terrainHeightMax { 5.0f };
constexpr float terrainHeightScalingFactor { terrainChunkSizeInPixel / 4.0f };

struct Chunk {
    unsigned short x, y;
    float height;
    float heightCenter;
    float heightCorners[4];
};

// This class describes the terrain of the play field devided into chunks.
// Each chunk has a position and a height value associated, resulting in slopes from one chunk to
// another. Slopes will have an effect on travel speed as well as on line of sight and fire range
// for units. Too steep slopes will result in a blocked path, thus the unit will not be able to
// travel to a neigbouring chunk.
class Terrain {
    using Grid = std::array<Chunk, terrainWidthInChunks * terrainHeightInChunks>;
    Grid m_chunks;

public:
    static jt::Vector2f getMappedFieldPosition(jt::Vector2f const& pos);

    // TODO share map information from server to clients
    Terrain(std::string const& mapFilename = "assets/maps/map_de_dust_2.json");

    Grid const& getChunks() const { return m_chunks; }

    Chunk const& getChunk(int x, int y) const;
    float getChunkHeight(int x, int y) const;
    float getSlopeAt(jt::Vector2f const& pos, jt::Vector2f const& dir) const;
    float getFieldHeight(jt::Vector2f const& pos) const;
    void parseMapFromFilename(std::string const& fileName);
};

#endif // JAMTEMPLATE_TERRAIN_HPP
