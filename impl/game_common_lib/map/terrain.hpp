#ifndef JAMTEMPLATE_TERRAIN_HPP
#define JAMTEMPLATE_TERRAIN_HPP

#include <vector.hpp>
#include <array>
#include <string>

constexpr int terrainWidthPerPlayerInChunks { 16 };
constexpr int terrainHeightInChunksPerPlayerInChunks { 15 };
constexpr int terrainFlankWidthInChunks { 5 };
constexpr int terrainWidthInChunks { terrainWidthPerPlayerInChunks * 2 };
constexpr int terrainHeightInChunks { terrainHeightInChunksPerPlayerInChunks
    + terrainFlankWidthInChunks * 2 };
constexpr float terrainChunkSizeInPixel { 16.0f };
constexpr float terrainHeightMax { 5.0f };
constexpr float terrainHeightScalingFactor { terrainChunkSizeInPixel / 4.0f };

struct Chunk {
    unsigned short x, y;
    float height;
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
    // TODO share map information from server to clients
    Terrain();

    Grid const& getChunks() const { return m_chunks; }

    Chunk const& getChunk(int x, int y) const { return m_chunks[y * terrainWidthInChunks + x]; }

    float getChunkHeight(jt::Vector2f const& pos) const;
    float getSlopeAt(jt::Vector2f const& pos, jt::Vector2f const& dir) const;
    void parseMapFromFilename(std::string const& fileName);
};

#endif // JAMTEMPLATE_TERRAIN_HPP
