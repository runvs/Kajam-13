#include <map/terrain.hpp>
#include <math_helper.hpp>
#include <iostream>

namespace {

template <typename T, typename Fn>
void updateRectangle(
    T& chunks, Fn const& fn, unsigned short x, unsigned short y, unsigned short w, unsigned short h)
{
    for (unsigned short j { y }; j < (y + h); ++j) {
        auto const wOffset = j * terrainWidthInChunks;
        for (unsigned short i { x }; i < (x + w); ++i) {
            fn(chunks[wOffset + i]);
        }
    }
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
    // initialize chunk set
    for (unsigned short h { 0 }; h < terrainHeightInChunks; ++h) {
        auto wOffset = h * terrainWidthInChunks;
        for (unsigned short w { 0 }; w < terrainWidthInChunks; ++w) {
            m_chunks[wOffset + w] = { w, h, 0.0f };
        }
    }

    // setup map
    // TODO replace by map loading
    // Tower 1 P1
    updateRectangle(
        m_chunks, [](auto& c) { c.height += 1.0f; }, 8, 8, 1, 1);

    // Tower 2 P1
    updateRectangle(
        m_chunks, [](auto& c) { c.height += 1.0f; }, 8, 16, 1, 1);

    // Tower 3 P2
    updateRectangle(
        m_chunks, [](auto& c) { c.height += 1.0f; }, 24, 8, 1, 1);

    // Tower 4 P2
    updateRectangle(
        m_chunks, [](auto& c) { c.height += 1.0f; }, 24, 16, 1, 1);

    // Some mountain
    updateRectangle(
        m_chunks, [](auto& c) { c.height += 2.0f; }, 14, 10, 5, 5);
    updateRectangle(
        m_chunks, [](auto& c) { c.height += 1.0f; }, 15, 11, 3, 3);
    updateRectangle(
        m_chunks, [](auto& c) { c.height += 0.5f; }, 16, 12, 1, 1);

    // testing mountain heights
    updateRectangle(
        m_chunks, [](auto& c) { c.height += 1.0f; }, 1, 21, 3, 3);
    updateRectangle(
        m_chunks, [](auto& c) { c.height += 2.0f; }, 6, 21, 3, 3);
    updateRectangle(
        m_chunks, [](auto& c) { c.height += 3.0f; }, 11, 21, 3, 3);
    updateRectangle(
        m_chunks, [](auto& c) { c.height += 5.0f; }, 16, 21, 3, 3);

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
