#ifndef JAMTEMPLATE_TERRAIN_HPP
#define JAMTEMPLATE_TERRAIN_HPP

#include <nlohmann.hpp>
#include <vector2.hpp>
#include <array>
#include <functional>
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
    unsigned short x {}, y {};
    float height {};
    float heightCenter {};
    float heightCorners[4] {};
};

void to_json(nlohmann::json& j, Chunk const& v);
void from_json(nlohmann::json const& j, Chunk& v);

// This class describes the terrain of the play field devided into chunks.
// Each chunk has a position and a height value associated, resulting in slopes from one chunk to
// another. Slopes will have an effect on travel speed as well as on line of sight and fire range
// for units. Too steep slopes will result in a blocked path, thus the unit will not be able to
// travel to a neigbouring chunk.
class Terrain {
public:
    using Grid = std::array<Chunk, terrainWidthInChunks * terrainHeightInChunks>;
    using UpdateCallback = std::function<void()>;

    static jt::Vector2f getMappedFieldPosition(jt::Vector2f const& pos, int& x, int& y);

    explicit Terrain(std::string const& mapFilename);
    Terrain() = default;

    void from_json(nlohmann::json const& j);

    std::string const& getName() const { return m_name; }
    Grid const& getChunks() const { return m_chunks; }

    void setOnUpdate(UpdateCallback&& cb) const;
    Chunk const& getChunk(int x, int y) const;
    float getChunkHeight(int x, int y) const;
    float getSlopeAt(jt::Vector2f const& pos, jt::Vector2f const& dir) const;
    float getFieldHeight(jt::Vector2f const& pos) const;

private:
    std::string m_name;
    Grid m_chunks;
    UpdateCallback mutable m_updateCallback;

    void setChunks(Grid const& grid);
};

void to_json(nlohmann::json& j, Terrain const& v);

#endif // JAMTEMPLATE_TERRAIN_HPP
