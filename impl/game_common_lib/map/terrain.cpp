#define _USE_MATH_DEFINES
#include <cmath>

#include "vector.hpp"
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
    j = { { "player", v.playerId }, { "x", v.position.x }, { "y", v.position.y } };
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
    j = { { "heights", v.heights }, { "towers", v.towers } };
}

void from_json(nlohmann::json const& j, Map& v)
{
    j.at("heights").get_to(v.heights);
    j.at("towers").get_to(v.towers);
}

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

    float length() const { return std::sqrt(x * x + y * y + z * z); }

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

    bool isZero() const { return x == 0 && y == 0 && z == 0; }
};

template <typename T>
Vector3f getNormalOfTriangleFor(T const& chunks, jt::Vector2f const& pos)
{
    int posX, posY;
    if (!posToCoord(pos, posX, posY)) {
        return {};
    }
    auto const& chunk = chunks[coordToIndex(posX, posY)];
    auto const posHeight = chunk.heightCenter;
    auto const posCenter = Terrain::getMappedFieldPosition(pos);

    if (posCenter == pos) {
        return {};
    }

    // get closest triangle
    auto const getCornerPos = [](int x, int y, int i) {
        jt::Vector2f p { 1.0f * x * terrainChunkSizeInPixel, 1.0f * y * terrainChunkSizeInPixel };
        if (i % 2) {
            p.x += terrainChunkSizeInPixel;
        }
        if (i > 1) {
            p.y += terrainChunkSizeInPixel;
        }
        return p;
    };

    int offX = static_cast<int>(pos.x) % terrainChunkSizeInPixel;
    int offY = static_cast<int>(pos.y) % terrainChunkSizeInPixel;
    if ((offY >= offX) && (offY < (terrainChunkSizeInPixel - offX))) {
        // left triangle
        auto const cTl = getCornerPos(posX, posY, 0);
        auto const cBl = getCornerPos(posX, posY, 2);
        auto const r = Vector3f { cBl - cTl, chunk.heightCorners[2] - chunk.heightCorners[0] };
        auto const s = Vector3f { posCenter - cTl, posHeight - chunk.heightCorners[0] };
        return s.crossProduct(r).normalized();
    } else if ((offY >= (terrainChunkSizeInPixel - offX)) && (offY < offX)) {
        // right triangle
        auto const cTr = getCornerPos(posX, posY, 1);
        auto const cBr = getCornerPos(posX, posY, 3);
        auto const r = Vector3f { cBr - cTr, chunk.heightCorners[3] - chunk.heightCorners[1] };
        auto const s = Vector3f { posCenter - cTr, posHeight - chunk.heightCorners[1] };
        return s.crossProduct(r).normalized();
    } else if ((offY < offX) && (offY < (terrainChunkSizeInPixel - offX))) {
        // top triangle
        auto const cTl = getCornerPos(posX, posY, 0);
        auto const cTr = getCornerPos(posX, posY, 1);
        auto const r = Vector3f { cTr - cTl, chunk.heightCorners[1] - chunk.heightCorners[0] };
        auto const s = Vector3f { posCenter - cTl, posHeight - chunk.heightCorners[0] };
        return s.crossProduct(r).normalized();
    } else if ((offY >= offX) && (offY >= (terrainChunkSizeInPixel - offX))) {
        // bot triangle
        auto const cBr = getCornerPos(posX, posY, 3);
        auto const cBl = getCornerPos(posX, posY, 2);
        auto const r = Vector3f { cBl - cBr, chunk.heightCorners[2] - chunk.heightCorners[3] };
        auto const s = Vector3f { posCenter - cBr, posHeight - chunk.heightCorners[3] };
        return s.crossProduct(r).normalized();
    }

    // may not happen
    assert(false);
    return {};
}

} // namespace

Terrain::Terrain(std::string const& mapFilename) { parseMapFromFilename(mapFilename); }

Chunk const& Terrain::getChunk(int x, int y) const
{
    auto idx = coordToIndex(x, y);
    assert(idx <= m_chunks.size());
    return m_chunks[idx];
}

float Terrain::getChunkHeight(int x, int y) const { return getChunk(x, y).heightCenter; }

float Terrain::getSlopeAt(jt::Vector2f const& pos, jt::Vector2f const& dir) const
{
    Vector3f const posDir { pos + dir };
    Vector3f const normal { getNormalOfTriangleFor(m_chunks, pos) };
    if (normal.isZero() || (normal.x == 0 && normal.y == 0)) {
        return 0.0f;
    }
    auto const projectionOffset = normal * (normal.dot(posDir) / normal.dot(normal));
    auto const posDirProjected = posDir - projectionOffset;
    return static_cast<float>(acosf(posDirProjected.z / posDirProjected.length()) * 90.0f / M_PI);
}

jt::Vector2f Terrain::getMappedFieldPosition(jt::Vector2f const& pos)
{
    return { pos.x - static_cast<int>(pos.x) % terrainChunkSizeInPixel
            + terrainChunkSizeInPixel / 2,
        pos.y - static_cast<int>(pos.y) % terrainChunkSizeInPixel + terrainChunkSizeInPixel / 2 };
}

float Terrain::getFieldHeight(jt::Vector2f const& pos) const
{
    int posX, posY;
    if (!posToCoord(pos, posX, posY)) {
        return 0.0f;
    }
    auto const& chunk = getChunk(posX, posY);
    auto const posHeight = chunk.heightCenter;
    auto const posCenter = getMappedFieldPosition(pos);

    if (posCenter == pos) {
        return posHeight;
    }

    Vector3f const p { pos, posHeight };
    Vector3f const pv { posCenter - pos, posHeight };
    auto const normal = getNormalOfTriangleFor(m_chunks, pos);
    if (normal.isZero()) {
        return posHeight;
    }
    auto const ray = Vector3f { 0.0f, 0.0f, 1.0f }.normalized() - p;
    return pv.dot(normal) / normal.dot(ray);
}

void Terrain::parseMapFromFilename(std::string const& fileName)
{
    std::ifstream fileStream { fileName };
    auto const map = nlohmann::json::parse(fileStream).get<Map>();
    for (unsigned short y { 0 }; y < terrainHeightInChunks; ++y) {
        for (unsigned short x { 0 }; x < terrainWidthInChunks; ++x) {
            auto const height
                = jt::MathHelper::clamp(map.heights[coordToIndex(x, y)], 0.0f, terrainHeightMax);
            auto& chunk = m_chunks[coordToIndex(x, y)]
                = { x, y, height, height, { height, height, height, height } };
            // adjust height offsets while we load up the chunks
            if (y > 0) {
                auto& top = m_chunks[coordToIndex(x, y - 1)];
                if (top.height < chunk.height) {
                    if (top.heightCenter > top.height) {
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
                    chunk.heightCorners[0] = top.heightCorners[2];
                }
            }
            if (x > 0) {
                auto& left = m_chunks[coordToIndex(x - 1, y)];
                if (left.height < chunk.height) {
                    if (left.heightCenter > left.height) {
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
                    left.heightCorners[1] = chunk.heightCorners[0];
                }
            }
            if (y > 0) {
                auto& top = m_chunks[coordToIndex(x, y - 1)];
                top.heightCorners[2] = chunk.heightCorners[0];
                if (x > 0) {
                    auto& topLeft = m_chunks[coordToIndex(x - 1, y - 1)];
                    topLeft.heightCorners[3] = chunk.heightCorners[0];
                }
            }
        }
    }
}
