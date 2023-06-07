#include <game_interface.hpp>
#include <game_properties.hpp>
#include <line.hpp>
#include <map/terrain_renderer.hpp>
#include <math_helper.hpp>
#include <sprite.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <cmath>
#include <sstream>

namespace {

constexpr int terrainColorGradient[] = { 0, 0, 0 };
constexpr auto terrainColorGradientMax = 255 - terrainColorGradient[0];
constexpr std::size_t midIndices[] { 2, 4, 7, 10 };
constexpr std::size_t leftIndices[] { 0, 3, 5, 6 };
constexpr std::size_t topIndices[] { 0, 1, 3, 11 };
constexpr std::size_t rightIndices[] { 1, 8, 9, 11 };
constexpr std::size_t botIndices[] { 5, 6, 8, 9 };
constexpr std::size_t topLeftIndices[] { 0, 3 };
constexpr std::size_t topRightIndices[] { 1, 11 };
constexpr std::size_t botLeftIndices[] { 5, 6 };
constexpr std::size_t botRightIndices[] { 8, 9 };
constexpr auto const chunkSize = terrainChunkSizeInPixel;
constexpr auto const chunkSizeHalf = chunkSize / 2.0f;
constexpr float vertexHeightOffsetMap[] { 0, 0, chunkSizeHalf, 0, chunkSizeHalf, chunkSize,
    chunkSize, chunkSizeHalf, chunkSize, chunkSize, chunkSizeHalf, 0 };

float getTerrainHeight(int y, float height)
{
    height = jt::MathHelper::clamp(height, 0.0f, terrainHeightMax);
    return y * terrainChunkSizeInPixel - height * terrainHeightScalingFactor;
}

sf::Color getTerrainColor(float height)
{
    height = jt::MathHelper::clamp(height, 0.0f, terrainHeightMax);
    sf::Color terrainColor { terrainColorGradient[0], terrainColorGradient[1],
        terrainColorGradient[2] };
    auto const interPolatedHeightColorOffset
        = static_cast<int>(terrainColorGradientMax * height / terrainHeightMax);
    terrainColor.r = std::min(255, terrainColor.r + interPolatedHeightColorOffset);
    terrainColor.g = std::min(255, terrainColor.g + interPolatedHeightColorOffset);
    terrainColor.b = std::min(255, terrainColor.b + interPolatedHeightColorOffset);
    return terrainColor;
}

template <typename T>
Chunk const& getChunk(T const& chunks, int x, int y)
{
    return chunks[y * terrainWidthInChunks + x];
}

template <typename T, typename I>
void updateVertices(T& vertices, I const& indices, float height, sf::Color const& color)
{
    for (auto const idx : indices) {
        vertices[idx].position.y = height + vertexHeightOffsetMap[idx];
        vertices[idx].color = color;
    }
}

template <typename G, typename T>
void checkLeft(G& grid, T const& chunks, Chunk const& chunk)
{
    // nothing to the left
    if (chunk.x == 0) {
        return;
    }

    auto const& left = getChunk(chunks, chunk.x - 1, chunk.y);

    // left one lower
    if (left.height < chunk.height) {
        auto const height = getTerrainHeight(chunk.y, chunk.height);
        auto const heightHalf = (height + getTerrainHeight(left.y, left.height)) / 2;
        auto const color = getTerrainColor(chunk.height);
        auto const colorHalf = getTerrainColor(chunk.height - (chunk.height - left.height) / 2);
        auto& vertices = grid[left.y * terrainWidthInChunks + left.x];
        updateVertices(vertices, midIndices, heightHalf, colorHalf);
        updateVertices(vertices, rightIndices, height, color);
    }
    // left one higher
    else if (left.height > chunk.height) {
        auto const height = getTerrainHeight(left.y, left.height);
        auto const heightHalf = (height + getTerrainHeight(chunk.y, chunk.height)) / 2;
        auto const color = getTerrainColor(left.height);
        auto const colorHalf = getTerrainColor(left.height - (left.height - chunk.height) / 2);
        auto& vertices = grid[chunk.y * terrainWidthInChunks + chunk.x];
        updateVertices(vertices, leftIndices, height, color);
        updateVertices(vertices, midIndices, heightHalf, colorHalf);

        // corner case
        if ((chunk.x > 0) && (chunk.y > 0)) {
            auto const& leftTop = getChunk(chunks, chunk.x - 1, chunk.y - 1);
            auto const& top = getChunk(chunks, chunk.x, chunk.y - 1);
            if (leftTop.height == top.height) {
                auto const heightHalf = (height + getTerrainHeight(top.y, top.height)) / 2;
                auto& verticesTop = grid[top.y * terrainWidthInChunks + top.x];
                updateVertices(
                    verticesTop, midIndices, heightHalf - terrainChunkSizeInPixel / 2, colorHalf);
                updateVertices(
                    verticesTop, botLeftIndices, height - terrainChunkSizeInPixel, color);
            }
        }
    }
}

template <typename G, typename T>
void checkTop(G& grid, T const& chunks, Chunk const& chunk)
{
    // nothing up top
    if (chunk.y == 0) {
        return;
    }

    auto const& top = getChunk(chunks, chunk.x, chunk.y - 1);

    // top one lower
    if (top.height < chunk.height) {
        auto const height = getTerrainHeight(chunk.y, chunk.height);
        auto const heightHalf = (height + getTerrainHeight(top.y, top.height)) / 2;
        auto const color = getTerrainColor(chunk.height);
        auto const colorHalf = getTerrainColor(chunk.height - (chunk.height - top.height) / 2);
        auto& vertices = grid[top.y * terrainWidthInChunks + top.x];
        updateVertices(vertices, midIndices, heightHalf - terrainChunkSizeInPixel / 2, colorHalf);
        updateVertices(vertices, botIndices, height - terrainChunkSizeInPixel, color);

        // corner case
        if ((chunk.x > 0) && (chunk.y > 0)) {
            auto const& left = getChunk(chunks, chunk.x - 1, chunk.y);
            auto const& leftTop = getChunk(chunks, chunk.x - 1, chunk.y - 1);
            if (left.height == leftTop.height) {
                auto& verticesLeftTop = grid[leftTop.y * terrainWidthInChunks + leftTop.x];
                updateVertices(verticesLeftTop, midIndices,
                    heightHalf - terrainChunkSizeInPixel / 2, colorHalf);
                updateVertices(
                    verticesLeftTop, botRightIndices, height - terrainChunkSizeInPixel, color);
            }
        }
    }
    // top one higher
    else if (top.height > chunk.height) {
        auto const height = getTerrainHeight(top.y, top.height);
        auto const heightHalf = (height + getTerrainHeight(chunk.y, chunk.height)) / 2;
        auto const color = getTerrainColor(top.height);
        auto const colorHalf = getTerrainColor(top.height - (top.height - chunk.height) / 2);
        auto& vertices = grid[chunk.y * terrainWidthInChunks + chunk.x];
        updateVertices(vertices, topIndices, height + terrainChunkSizeInPixel, color);
        updateVertices(vertices, midIndices, heightHalf + terrainChunkSizeInPixel / 2, colorHalf);

        // corner case
        if ((chunk.x > 0) && (chunk.y > 0)) {
            auto const& left = getChunk(chunks, chunk.x - 1, chunk.y);
            auto const& leftTop = getChunk(chunks, chunk.x - 1, chunk.y - 1);
            if (left.height == leftTop.height) {
                auto& verticesLeft = grid[left.y * terrainWidthInChunks + left.x];
                updateVertices(
                    verticesLeft, topRightIndices, height + terrainChunkSizeInPixel, color);
                updateVertices(
                    verticesLeft, midIndices, heightHalf + terrainChunkSizeInPixel / 2, colorHalf);
            }
        }
    } else if ((chunk.x > 0) && (chunk.y > 0)) {
        auto const& left = getChunk(chunks, chunk.x - 1, chunk.y);
        auto const& leftTop = getChunk(chunks, chunk.x - 1, chunk.y - 1);
        if (left.height < leftTop.height) {
            auto const height = getTerrainHeight(leftTop.y, leftTop.height);
            auto const heightHalf = (height + getTerrainHeight(chunk.y, chunk.height)) / 2;
            auto const color = getTerrainColor(leftTop.height);
            auto const colorHalf
                = getTerrainColor(leftTop.height - (leftTop.height - chunk.height) / 2);
            auto& vertices = grid[chunk.y * terrainWidthInChunks + chunk.x];
            updateVertices(vertices, topLeftIndices, height + terrainChunkSizeInPixel, color);
            updateVertices(
                vertices, midIndices, heightHalf + terrainChunkSizeInPixel / 2, colorHalf);
        }
    }
}

void drawTerrainGrid(sf::RenderTexture& texture)
{
    sf::Color const colorGrid { 22, 22, 22 };

    sf::VertexArray linesHorizontal { sf::Lines, 2 * terrainHeightInChunks - 2 };
    for (unsigned short h { 1 }; h < terrainHeightInChunks; ++h) {
        linesHorizontal[(h - 1) * 2] = { { 0, h * terrainChunkSizeInPixel }, colorGrid };
        linesHorizontal[(h - 1) * 2 + 1]
            = { { terrainWidthInChunks * terrainChunkSizeInPixel, h * terrainChunkSizeInPixel },
                  colorGrid };
    }
    texture.draw(linesHorizontal);

    sf::VertexArray linesVertical { sf::Lines, 2 * terrainWidthInChunks - 2 };
    for (unsigned short w { 1 }; w < terrainWidthInChunks; ++w) {
        linesVertical[(w - 1) * 2] = { { w * terrainChunkSizeInPixel, 0 }, colorGrid };
        linesVertical[(w - 1) * 2 + 1]
            = { { w * terrainChunkSizeInPixel, terrainHeightInChunks * terrainChunkSizeInPixel },
                  colorGrid };
    }
    texture.draw(linesVertical);
}

} // namespace

struct TerrainRenderer::Private {
    bool drawGrid {};
    sf::RenderTexture texture {};
    std::unique_ptr<jt::Sprite> sprite {};
};

TerrainRenderer::TerrainRenderer(Terrain const& t)
    : m { std::make_shared<TerrainRenderer::Private>() }
    , m_terrain { t }
{
}

void TerrainRenderer::setDrawGrid(bool v) { m->drawGrid = v; }

void TerrainRenderer::doCreate()
{
    // create chunk to vertex map
    std::array<sf::VertexArray, terrainWidthInChunks * terrainHeightInChunks> grid;
    auto const& chunks = m_terrain.getChunks();
    for (unsigned short h { 0 }; h < terrainHeightInChunks; ++h) {
        auto const wOffset = h * terrainWidthInChunks;
        for (unsigned short w { 0 }; w < terrainWidthInChunks; ++w) {
            auto const& chunk = chunks[wOffset + w];
            auto const posX = w * chunkSize;
            auto const posY = getTerrainHeight(h, chunk.height);
            auto const color = getTerrainColor(chunk.height);

            // draw chunk triangles
            auto& vertices = grid[wOffset + w] = sf::VertexArray { sf::Triangles, 12 };

            // top triangle
            vertices[0] = { { posX, posY }, color };
            vertices[1] = { { posX + chunkSize, posY }, color };
            vertices[2] = { { posX + chunkSizeHalf, posY + chunkSizeHalf }, color };

            // left triangle
            vertices[3] = { { posX, posY }, color };
            vertices[4] = { { posX + chunkSizeHalf, posY + chunkSizeHalf }, color };
            vertices[5] = { { posX, posY + chunkSize }, color };

            // bot triangle
            vertices[6] = { { posX, posY + chunkSize }, color };
            vertices[7] = { { posX + chunkSizeHalf, posY + chunkSizeHalf }, color };
            vertices[8] = { { posX + chunkSize, posY + chunkSize }, color };

            // right triangle
            vertices[9] = { { posX + chunkSize, posY + chunkSize }, color };
            vertices[10] = { { posX + chunkSizeHalf, posY + chunkSizeHalf }, color };
            vertices[11] = { { posX + chunkSize, posY }, color };

            // update neighbouring chunks
            checkLeft(grid, chunks, chunk);
            checkTop(grid, chunks, chunk);
        }
    }

    // draw terrain texture from vertices
    bool result { m->texture.create(static_cast<unsigned int>(GP::GetScreenSize().x),
        static_cast<unsigned int>(GP::GetScreenSize().y)) };
    // TODO check error
    (void)result;
    m->texture.clear(getTerrainColor(0.0f));
    m->texture.setSmooth(true);
    for (auto const& e : grid) {
        m->texture.draw(e);
    }
    if (m->drawGrid) {
        drawTerrainGrid(m->texture);
    }
    m->texture.display();

    // sprite with rendered terrain texture to display
    m->sprite = std::make_unique<jt::Sprite>();
    m->sprite->setIgnoreCamMovement(true);
    m->sprite->setPosition(jt::Vector2f(0.0f, 0.0f));
    m->sprite->fromTexture(m->texture.getTexture());
    m->sprite->update(0.0f);
}

void TerrainRenderer::doUpdate(float const elapsed) { m->sprite->update(elapsed); }

void TerrainRenderer::doDraw() const { m->sprite->draw(renderTarget()); }
