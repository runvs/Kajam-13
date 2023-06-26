#define _USE_MATH_DEFINES
#include <cmath>

#include "terrain.hpp"
#include <json_keys.hpp>
#include <math_helper.hpp>
#include <vector.hpp>
#include <fstream>
#include <vector>

namespace {

std::size_t coordToIndex(int x, int y) { return x + y * terrainWidthInChunks; }

bool posToCoord(jt::Vector2f const& pos, int& x, int& y)
{
    x = static_cast<int>(pos.x / terrainChunkSizeInPixel);
    y = static_cast<int>(pos.y / terrainChunkSizeInPixel);
    return ((x >= 0) && (x < terrainWidthInChunks) && (y >= 0) && (y < terrainHeightInChunks));
}

struct Vector3f {
    float x, y, z;

    Vector3f() = default;

    Vector3f(float x, float y, float z)
        : x { x }
        , y { y }
        , z { z }
    {
    }

    Vector3f(jt::Vector2f const& v)
        : x { v.x }
        , y { v.y }
        , z {}
    {
    }

    Vector3f(jt::Vector2f const& v, float z)
        : x { v.x }
        , y { v.y }
        , z { z }
    {
    }

    Vector3f operator+(Vector3f const& v) const { return { x + v.x, y + v.y, z + v.z }; }
    Vector3f operator-(Vector3f const& v) const { return { x - v.x, y - v.y, z - v.z }; }
    Vector3f operator*(float const v) const { return { x * v, y * v, z * v }; }

    float determinant() const { return x * x + y * y + z * z; }
    float length() const { return std::sqrt(determinant()); }

    Vector3f normalized() const
    {
        auto const len = length();
        if (len != 0.0f) {
            return { x / len, y / len, z / len };
        }
        return {};
    }

    float dot(Vector3f const& v) const { return x * v.x + y * v.y + z * v.z; }

    Vector3f crossProduct(Vector3f const& v) const
    {
        return { y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x };
    }

    float angleRad(Vector3f const& v) const
    {
        return acosf(dot(v) / std::sqrt(determinant() * v.determinant()));
    }

    float angleDeg(Vector3f const& v) const
    {
        return static_cast<float>(angleRad(v) * 180.0f / M_PI);
    }

    bool isZero() const { return x == 0 && y == 0 && z == 0; }
};

template <typename T>
Vector3f getNormalOfTriangleForPosition(
    T const& chunks, jt::Vector2f const& pos, bool const convertHeight)
{
    int posX, posY;
    if (!posToCoord(pos, posX, posY)) {
        return {};
    }
    auto const& chunk = chunks[coordToIndex(posX, posY)];
    auto const posHeight = chunk.heightCenter;

    // the height of the map is ranged [0;terrainHeightMax] and thus needs to be translated into a
    // value that can be used for an actual slope calculation when doing vector arithmetics
    auto const convertHeightToZ = [&](auto const v) {
        if (convertHeight) {
            // conversion formula to map values of [0;terrainHeightMax] to degrees as follows:
            // 0: 0 | 0.5: 14.0362 | 1: 26.565 | 1.5: 36.8699 | 2: 45 | 3: 56.3099 | 4: 63.4349 |
            // 5: 68.19
            return static_cast<float>(
                v * 0.25 * terrainChunkSizeInPixel * 2 /*shift to center factor*/);
        }
        return v;
    };

    // use field center as origin for vector calculations, e.g. top left corner is [-8;-8]
    Vector3f const c { 0, 0, convertHeightToZ(posHeight) };
    auto const minXY = -terrainChunkSizeInPixelHalf;
    auto const maxXY = terrainChunkSizeInPixelHalf - 1; // TODO check whether pixel 16 is an issue
    Vector3f const tl { minXY, minXY, convertHeightToZ(chunk.heightCorners[0]) };
    Vector3f const tr { maxXY, minXY, convertHeightToZ(chunk.heightCorners[1]) };
    Vector3f const bl { minXY, maxXY, convertHeightToZ(chunk.heightCorners[2]) };
    Vector3f const br { maxXY, maxXY, convertHeightToZ(chunk.heightCorners[3]) };

    // determine which triangle the position is in and calculate normal vector for it
    int offX = static_cast<int>(pos.x) % terrainChunkSizeInPixel;
    int offY = static_cast<int>(pos.y) % terrainChunkSizeInPixel;
    // left triangle
    if ((offY >= offX) && (offY < (terrainChunkSizeInPixel - offX))) {
        return (c - bl).crossProduct(c - tl).normalized();
    }
    // right triangle
    if ((offY < offX) && (offY >= (terrainChunkSizeInPixel - offX))) {
        return (c - tr).crossProduct(c - br).normalized();
    }
    // top triangle
    if ((offY < offX) && (offY < (terrainChunkSizeInPixel - offX))) {
        return (c - tl).crossProduct(c - tr).normalized();
    }
    // bot triangle
    if ((offY >= offX) && (offY >= (terrainChunkSizeInPixel - offX))) {
        return (c - br).crossProduct(c - bl).normalized();
    }

    // may not happen
    assert(false);
    return {};
}

} // namespace

void to_json(nlohmann::json& j, Chunk const& v) { j = v.height; }

void from_json(nlohmann::json const& j, Chunk& v) { j.get_to(v.height); }

jt::Vector2f Terrain::getMappedFieldPosition(jt::Vector2f const& pos, int& x, int& y)
{
    if (!posToCoord(pos, x, y)) {
        return {};
    }
    return jt::Vector2f { static_cast<float>(
                              x * terrainChunkSizeInPixel + terrainChunkSizeInPixelHalf),
        static_cast<float>(y * terrainChunkSizeInPixel + terrainChunkSizeInPixelHalf) };
}

Terrain::Terrain(std::string const& mapFilename)
{
    from_json(nlohmann::json::parse(std::ifstream { mapFilename }));
}

void Terrain::from_json(nlohmann::json const& j)
{
    j.at(jk::mapName).get_to(m_name);
    setChunks(j.at(jk::mapHeights).get<Grid>());
}

void Terrain::setOnUpdate(UpdateCallback&& cb) const
{
    m_updateCallback = std::forward<UpdateCallback>(cb);
}

Chunk const& Terrain::getChunk(int x, int y) const
{
    auto idx = coordToIndex(x, y);
    assert(idx < m_chunks.size());
    return m_chunks[idx];
}

float Terrain::getChunkHeight(int x, int y) const { return getChunk(x, y).heightCenter; }

float Terrain::getSlopeAt(jt::Vector2f const& pos, jt::Vector2f const& dir) const
{
    Vector3f const normal { getNormalOfTriangleForPosition(m_chunks, pos, true) };
    if (normal.isZero() || (normal.x == 0 && normal.y == 0)) {
        return 0.0f;
    }
    auto const v = Vector3f { dir };
    auto const np = normal * v.dot(normal);
    auto const vp = v - np;
    auto const slope = vp.angleDeg(v) * (v.x < 0 || v.y < 0 ? -1 : 1);
    return slope;
}

float Terrain::getFieldHeight(jt::Vector2f const& pos) const
{
    int x, y;
    auto const posCenter = getMappedFieldPosition(pos, x, y);
    auto const& chunk = getChunk(x, y);
    auto const posHeight = chunk.heightCenter;

    if (posCenter == pos) {
        return posHeight;
    }

    auto const normal = getNormalOfTriangleForPosition(m_chunks, pos, false);
    if (normal.isZero()) {
        return posHeight;
    }

    // plane defined by posCenter3d and normal
    // (poscenter - pos) * normal / (0,0,1) * normal
    Vector3f const posCenter3d { posCenter, posHeight };
    return (posCenter3d - pos).dot(normal) / Vector3f { 0, 0, 1 }.dot(normal);
}

void Terrain::setChunks(Grid const& grid)
{
    for (unsigned short y { 0 }; y < terrainHeightInChunks; ++y) {
        for (unsigned short x { 0 }; x < terrainWidthInChunks; ++x) {
            auto const height
                = jt::MathHelper::clamp(grid[coordToIndex(x, y)].height, 0.0f, terrainHeightMax);
            auto& chunk = m_chunks[coordToIndex(x, y)]
                = { x, y, height, height, { height, height, height, height } };
            // adjust height offsets while we load up the chunks
            if (y > 0) {
                auto& top = m_chunks[coordToIndex(x, y - 1)];
                if (top.height < chunk.height) {
                    if (top.heightCenter > top.height && y > 1) {
                        // bridge needed, since higher ground top of top
                        auto& topOfTop = m_chunks[coordToIndex(x, y - 2)];
                        if (topOfTop.height < chunk.height) {
                            top.heightCenter
                                = topOfTop.height + (chunk.height - topOfTop.height) / 2;
                        } else {
                            top.heightCenter = chunk.height + (topOfTop.height - chunk.height) / 2;
                        }
                    } else {
                        top.heightCenter = top.height + (chunk.height - top.height) / 2;
                    }
                    top.heightCorners[2] = chunk.heightCorners[0];
                    top.heightCorners[3] = chunk.heightCorners[1];
                } else if (top.height > chunk.height) {
                    chunk.heightCenter = chunk.height + (top.height - chunk.height) / 2;
                    chunk.heightCorners[0] = top.heightCorners[2];
                    chunk.heightCorners[1] = top.heightCorners[3];
                } else if (top.heightCenter > chunk.height) {
                    chunk.heightCenter = top.height + (chunk.height - top.height) / 2;
                    chunk.heightCorners[0] = top.heightCorners[2];
                    chunk.heightCorners[1] = top.heightCorners[3];
                }
            }
            if (x > 0) {
                auto& left = m_chunks[coordToIndex(x - 1, y)];
                if (left.height < chunk.height) {
                    if (left.heightCenter > left.height && x > 1) {
                        // bridge needed, since higher ground left of left
                        auto& leftOfLeft = m_chunks[coordToIndex(x - 2, y)];
                        if (leftOfLeft.height < chunk.height) {
                            left.heightCenter
                                = leftOfLeft.height + (chunk.height - leftOfLeft.height) / 2;
                        } else {
                            left.heightCenter
                                = chunk.height + (leftOfLeft.height - chunk.height) / 2;
                        }
                    } else {
                        left.heightCenter = left.height + (chunk.height - left.height) / 2;
                    }
                    left.heightCorners[1] = chunk.heightCorners[0];
                    left.heightCorners[3] = chunk.heightCorners[2];
                } else if (left.height > chunk.height) {
                    chunk.heightCenter = chunk.height + (left.height - chunk.height) / 2;
                    chunk.heightCorners[0] = left.heightCorners[1];
                    chunk.heightCorners[2] = left.heightCorners[3];
                } else if (left.heightCenter < chunk.heightCenter) {
                    chunk.heightCenter = left.height + (chunk.height - left.height) / 2;
                    left.heightCorners[1] = chunk.heightCorners[0];
                    left.heightCorners[3] = chunk.heightCorners[2];
                }
            }
            if (y > 0) {
                auto& top = m_chunks[coordToIndex(x, y - 1)];
                top.heightCorners[2] = chunk.heightCorners[0];
                top.heightCorners[3] = chunk.heightCorners[1];
                if (x > 0) {
                    auto& topLeft = m_chunks[coordToIndex(x - 1, y - 1)];
                    topLeft.heightCorners[1] = top.heightCorners[0];
                    topLeft.heightCorners[3] = top.heightCorners[2];
                }
            }
        }
    }

    // notify TerrainRenderer
    if (m_updateCallback) {
        m_updateCallback();
    }
}

void to_json(nlohmann::json& j, Terrain const& v)
{
    j[jk::mapName] = v.getName();
    j[jk::mapHeights] = v.getChunks();
}
