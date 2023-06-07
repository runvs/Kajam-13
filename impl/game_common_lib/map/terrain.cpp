#include "terrain.hpp"

namespace {

template <typename T, typename Fn>
void updateRectangle(
    T& chunks, Fn const& fn, unsigned short x, unsigned short y, unsigned short w, unsigned short h)
{
    for (unsigned short j { y }; j < (y + h); ++j) {
        auto wOffset = j * terrainWidthInChunks;
        for (unsigned short i { x }; i < (x + w); ++i) {
            fn(chunks[wOffset + i]);
        }
    }
}

} // namespace

Terrain::Terrain()
{
    for (unsigned short h { 0 }; h < terrainHeightInChunks; ++h) {
        auto wOffset = h * terrainWidthInChunks;
        for (unsigned short w { 0 }; w < terrainWidthInChunks; ++w) {
            m_chunks[wOffset + w] = { w, h, 0.0f };
        }
    }

    // Tower 1
    updateRectangle(
        m_chunks, [](auto& c) { c.height += 0.3f; }, 14, 22, 1, 1);

    // // Tower 2
    // updateRectangle(
    //     m_chunks, [](auto& c) { c.height += 0.3f; }, 14, 49, 1, 1);

    // // Tower 3
    // updateRectangle(
    //     m_chunks, [](auto& c) { c.height += 0.3f; }, 47, 22, 1, 1);

    // // Tower 4
    // updateRectangle(
    //     m_chunks, [](auto& c) { c.height += 0.3f; }, 47, 49, 1, 1);

    // // Some mountain
    // updateRectangle(
    //     m_chunks, [](auto& c) { c.height += 0.4f; }, 28, 14, 5, 5);
    // updateRectangle(
    //     m_chunks, [](auto& c) { c.height += 0.2f; }, 29, 15, 3, 3);
    // updateRectangle(
    //     m_chunks, [](auto& c) { c.height += 0.1f; }, 31, 17, 1, 1);

    // // Another mountain
    // updateRectangle(
    //     m_chunks, [](auto& c) { c.height += 0.4f; }, 8, 54, 5, 5);
    // updateRectangle(
    //     m_chunks, [](auto& c) { c.height += 0.2f; }, 9, 55, 3, 3);
    // updateRectangle(
    //     m_chunks, [](auto& c) { c.height += 0.1f; }, 11, 57, 1, 1);
}
