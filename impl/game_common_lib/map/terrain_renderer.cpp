#include "terrain_renderer.hpp"
#include <game_interface.hpp>
#include <game_properties.hpp>
#include <line.hpp>
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
constexpr std::size_t topLeftIndices[] { 1, 11 };
constexpr std::size_t topRightIndices[] { 0, 3 };
constexpr std::size_t botLeftIndices[] { 5, 6 };
constexpr std::size_t botRightIndices[] { 8, 9 };

float getTerrainHeightOffset(float height) { return height * terrainHeightScalingFactor; }

sf::Color getTerrainColor(float height)
{
    height = jt::MathHelper::clamp(height, 0.0f, terrainHeightMax);
    sf::Color terrainColor { terrainColorGradient[0], terrainColorGradient[1],
        terrainColorGradient[2] };
    auto const interPolatedHeightColorOffset
        = (int)(terrainColorGradientMax * height / terrainHeightMax);
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
void updateVertices(T& vertices, I const& indices, float offsetHeight, sf::Color const& color)
{
    for (auto const idx : indices) {
        vertices[idx].position.y -= offsetHeight;
        vertices[idx].color = color;
    }
}

template <typename G, typename T>
void checkLeftTop(G& grid, T const& chunks, Chunk const& chunk)
{
    // nothing to the left top
    if ((chunk.x == 0) || (chunk.y == 0)) {
        return;
    }

    // auto const& left = getChunk(chunks, chunk.x - 1, chunk.y);
    // auto const& leftTop = getChunk(chunks, chunk.x - 1, chunk.y);
    // auto& verticesLeft = grid[left.y * terrainWidthInChunks + left.x];
    // auto& verticesLeftTop = grid[leftTop.y * terrainWidthInChunks + leftTop.x];

    // // top one lower
    // if (verticesLeft[topRightIndices[0]].position.y
    //     > verticesLeftTop[botRightIndices[0]].position.y) {
    //     auto const offsetHeight = getTerrainHeightOffset(chunk.height);
    //     auto const offsetHeightHalf = offsetHeight / 2;
    //     auto const color = getTerrainColor(chunk.height);
    //     auto const colorHalf = getTerrainColor(chunk.height - (chunk.height - left.height) / 2);
    //     // corner
    //     updateVertices(verticesLeftTop, botRightIndices,
    //         verticesLeft[topRightIndices[0]].position.y, verticesLeft[topRightIndices[0]].color);
    //     // mid
    //     updateVertices(verticesLeftTop, botRightIndices,
    //         verticesLeft[topRightIndices[0]].position.y, verticesLeft[topRightIndices[0]].color);
    // }
    // // top one higher
    // else if (verticesLeft[topRightIndices[0]].position.y
    //     < verticesLeftTop[botRightIndices[0]].position.y) {
    // }
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
        auto& vertices = grid[left.y * terrainWidthInChunks + left.x];
        auto const offsetHeight = getTerrainHeightOffset(chunk.height);
        auto const offsetHeightHalf = offsetHeight / 2;
        auto const color = getTerrainColor(chunk.height);
        auto const colorHalf = getTerrainColor(chunk.height - (chunk.height - left.height) / 2);
        // mid
        updateVertices(vertices, midIndices, offsetHeightHalf, colorHalf);
        // right
        updateVertices(vertices, rightIndices, offsetHeight, color);
    }
    // left one higher
    else if (left.height > chunk.height) {
        auto& vertices = grid[chunk.y * terrainWidthInChunks + chunk.x];
        auto const offsetHeight = getTerrainHeightOffset(left.height);
        auto const offsetHeightHalf = offsetHeight / 2;
        auto const color = getTerrainColor(left.height);
        auto const colorHalf = getTerrainColor(left.height - (left.height - chunk.height) / 2);
        // mid
        updateVertices(vertices, midIndices, offsetHeightHalf, colorHalf);
        // left
        updateVertices(vertices, leftIndices, offsetHeight, color);
    } else {
        return;
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
        auto& vertices = grid[top.y * terrainWidthInChunks + top.x];
        auto const offsetHeight = getTerrainHeightOffset(chunk.height);
        auto const offsetHeightHalf = offsetHeight / 2;
        auto const color = getTerrainColor(chunk.height);
        auto const colorHalf = getTerrainColor(chunk.height - (chunk.height - top.height) / 2);
        // mid
        updateVertices(vertices, midIndices, offsetHeightHalf, colorHalf);
        // bot
        updateVertices(vertices, botIndices, offsetHeight, color);
    }
    // top one higher
    else if (top.height > chunk.height) {
        auto& vertices = grid[chunk.y * terrainWidthInChunks + chunk.x];
        auto const offsetHeight = getTerrainHeightOffset(top.height);
        auto const offsetHeightHalf = offsetHeight / 2;
        auto const color = getTerrainColor(top.height);
        auto const colorHalf = getTerrainColor(top.height - (top.height - chunk.height) / 2);
        // mid
        updateVertices(vertices, midIndices, offsetHeightHalf, colorHalf);
        // top
        updateVertices(vertices, topIndices, offsetHeight, color);
    } else {
        return;
    }

    // changes were made, so this potentially affects left / leftTop as well
    checkLeftTop(grid, chunks, chunk);
}

} // namespace

struct TerrainRenderer::Private {
    sf::RenderTexture texture {};
    std::unique_ptr<jt::Sprite> sprite {};
};

TerrainRenderer::TerrainRenderer(Terrain const& t)
    : m { std::make_shared<TerrainRenderer::Private>() }
    , m_terrain { t }
{
}

void TerrainRenderer::doCreate()
{
    // draw custom texture
    bool result = m->texture.create(static_cast<unsigned int>(GP::GetScreenSize().x),
        static_cast<unsigned int>(GP::GetScreenSize().y));
    // TODO check error
    (void)result;
    m->texture.clear(getTerrainColor(0.0f));
    m->texture.setSmooth(true);

    // border always straight
    sf::Color const colorBorder { 255, 255, 0 };
    float aliasOffsetX = 1.0f;
    sf::VertexArray lines { sf::LinesStrip, 5 };
    lines[0] = sf::Vertex { sf::Vector2f { aliasOffsetX, 0.0f }, colorBorder };
    lines[1] = sf::Vertex {
        sf::Vector2f { terrainWidthInChunks * terrainChunkSizeInPixel + aliasOffsetX, 0.0f },
        colorBorder
    };
    lines[2]
        = sf::Vertex { sf::Vector2f { terrainWidthInChunks * terrainChunkSizeInPixel + aliasOffsetX,
                           terrainHeightInChunks * terrainChunkSizeInPixel },
              colorBorder };
    lines[3] = sf::Vertex {
        sf::Vector2f { aliasOffsetX, terrainHeightInChunks * terrainChunkSizeInPixel }, colorBorder
    };
    lines[4] = sf::Vertex { sf::Vector2f { aliasOffsetX, 0.0f }, colorBorder };
    m->texture.draw(lines);

    auto const& chunks = m_terrain.getChunks();
    std::array<sf::VertexArray, terrainWidthInChunks * terrainHeightInChunks> grid;
    for (unsigned short h { 0 }; h < terrainHeightInChunks; ++h) {
        auto wOffset = h * terrainWidthInChunks;
        for (unsigned short w { 0 }; w < terrainWidthInChunks; ++w) {
            auto const& chunk = chunks[wOffset + w];
            grid[wOffset + w] = sf::VertexArray { sf::Triangles, 12 };
            auto& vertices = grid[wOffset + w];
            auto const posX = w * terrainChunkSizeInPixel;
            auto const posY = h * terrainChunkSizeInPixel;
            auto const offsetHeight = getTerrainHeightOffset(chunk.height);
            auto color = getTerrainColor(chunk.height);
            // top triangle
            vertices[0]
                = sf::Vertex { sf::Vector2f { posX + 0.0f, posY + 0.0f - offsetHeight }, color };
            vertices[1] = sf::Vertex {
                sf::Vector2f { posX + terrainChunkSizeInPixel, posY + 0.0f - offsetHeight }, color
            };
            vertices[2] = sf::Vertex { sf::Vector2f { posX + terrainChunkSizeInPixel / 2.0f,
                                           posY + terrainChunkSizeInPixel / 2.0f - offsetHeight },
                color };
            // left triangle
            vertices[3]
                = sf::Vertex { sf::Vector2f { posX + 0.0f, posY + 0.0f - offsetHeight }, color };
            vertices[4] = sf::Vertex { sf::Vector2f { posX + terrainChunkSizeInPixel / 2.0f,
                                           posY + terrainChunkSizeInPixel / 2.0f - offsetHeight },
                color };
            vertices[5] = sf::Vertex {
                sf::Vector2f { posX + 0.0f, posY + terrainChunkSizeInPixel - offsetHeight }, color
            };
            // bot triangle
            vertices[6] = sf::Vertex {
                sf::Vector2f { posX + 0.0f, posY + terrainChunkSizeInPixel - offsetHeight }, color
            };
            vertices[7] = sf::Vertex { sf::Vector2f { posX + terrainChunkSizeInPixel / 2.0f,
                                           posY + terrainChunkSizeInPixel / 2.0f - offsetHeight },
                color };
            vertices[8] = sf::Vertex { sf::Vector2f { posX + terrainChunkSizeInPixel,
                                           posY + terrainChunkSizeInPixel - offsetHeight },
                color };
            // right triangle
            vertices[9] = sf::Vertex { sf::Vector2f { posX + terrainChunkSizeInPixel,
                                           posY + terrainChunkSizeInPixel - offsetHeight },
                color };
            vertices[10] = sf::Vertex { sf::Vector2f { posX + terrainChunkSizeInPixel / 2.0f,
                                            posY + terrainChunkSizeInPixel / 2.0f - offsetHeight },
                color };
            vertices[11] = sf::Vertex {
                sf::Vector2f { posX + terrainChunkSizeInPixel, posY + 0.0f - offsetHeight }, color
            };
            checkLeft(grid, chunks, chunk);
            checkTop(grid, chunks, chunk);
        }
    }
    for (auto const& e : grid) {
        m->texture.draw(e);
    }

    // finalize texture
    m->texture.display();

    // sprite with rendered texture for terrain
    m->sprite = std::make_unique<jt::Sprite>();
    m->sprite->setIgnoreCamMovement(true);
    m->sprite->setPosition(jt::Vector2f(0.0f, 0.0f));
    // jt::Vector2f scale { GP::GetZoom(), GP::GetZoom() };
    // std::stringstream ss_log;
    // ss_log << "TerrainRenderer.scale: " << scale.x << "," << scale.y << "\n";
    // getGame()->logger().warning(ss_log.str());
    // m->sprite->setScale(scale);
    m->sprite->fromTexture(m->texture.getTexture());
    m->sprite->update(0.0f);
}

void TerrainRenderer::doUpdate(float const elapsed) { m->sprite->update(elapsed); }

void TerrainRenderer::doDraw() const { m->sprite->draw(renderTarget()); }
