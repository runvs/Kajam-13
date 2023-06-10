#include <game_properties.hpp>
#include <math_helper.hpp>
#include <sprite.hpp>
#include <terrain_renderer.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <cmath>

namespace {

constexpr float const chunkSize { terrainChunkSizeInPixel };
constexpr auto const chunkSizeHalf = chunkSize / 2.0f;

// clang-format off
sf::Color colorMap[] {
    // dirt layer
    { 90, 45, 25 },
    // grassy layer
    { 15, 100, 10 }, { 13, 80, 10 },
    // mountain layer
    { 60, 60, 60 }, { 120, 120, 120 }, { 160, 160, 160 },
    // mountain top
    { 200, 200, 200 }
};
// clang-format on

float getTerrainHeight(int y, float height)
{
    return y * chunkSize - height * terrainHeightScalingFactor;
}

sf::Color getTerrainColor(float height)
{
    height = jt::MathHelper::clamp(height, 0.0f, terrainHeightMax);
    if (height < 0.4) {
        return colorMap[0];
    }
    if (height < 1.4) {
        return colorMap[1];
    }
    if (height < 2.4) {
        return colorMap[2];
    }
    if (height < 3.7) {
        return colorMap[3];
    }
    if (height < 4.2) {
        return colorMap[4];
    }
    if (height < 4.6) {
        return colorMap[5];
    }
    return colorMap[6];
}

void drawTerrainGrid(sf::RenderTexture& texture)
{
    sf::Color const colorGrid { 20, 20, 20, 30 };

    sf::VertexArray linesHorizontal { sf::Lines, 2 * terrainHeightInChunks - 2 };
    for (unsigned short h { 1 }; h < terrainHeightInChunks; ++h) {
        linesHorizontal[(h - 1) * 2] = { { 0, h * chunkSize }, colorGrid };
        linesHorizontal[(h - 1) * 2 + 1]
            = { { terrainWidthInChunks * chunkSize, h * chunkSize }, colorGrid };
    }
    texture.draw(linesHorizontal);

    sf::VertexArray linesVertical { sf::Lines, 2 * terrainWidthInChunks - 2 };
    for (unsigned short w { 1 }; w < terrainWidthInChunks; ++w) {
        linesVertical[(w - 1) * 2] = { { w * chunkSize, 0 }, colorGrid };
        linesVertical[(w - 1) * 2 + 1]
            = { { w * chunkSize, terrainHeightInChunks * chunkSize }, colorGrid };
    }
    texture.draw(linesVertical);
}

} // namespace

struct TerrainRenderer::Private {
    bool drawGrid {};
    sf::RenderTexture texture {};
    std::unique_ptr<jt::Sprite> sprite {};
    sf::RenderTexture textureGrid {};
    std::unique_ptr<jt::Sprite> spriteGrid {};
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
        bool const lastLine { h == terrainHeightInChunks - 1 };
        auto const wOffset = h * terrainWidthInChunks;
        for (unsigned short w { 0 }; w < terrainWidthInChunks; ++w) {
            auto const& chunk = chunks[wOffset + w];
            auto const posX = w * chunkSize;
            auto const posYc = getTerrainHeight(h, chunk.heightCenter);
            auto const posYtl = getTerrainHeight(h, chunk.heightCorners[0]);
            auto const posYtr = getTerrainHeight(h, chunk.heightCorners[1]);
            auto const posYbl = getTerrainHeight(h, lastLine ? 0 : chunk.heightCorners[2]);
            auto const posYbr = getTerrainHeight(h, lastLine ? 0 : chunk.heightCorners[3]);
            auto const colorC = getTerrainColor(chunk.heightCenter);
            auto const colorTl = getTerrainColor(chunk.heightCorners[0]);
            auto const colorTr = getTerrainColor(chunk.heightCorners[1]);
            auto const colorBl = getTerrainColor(chunk.heightCorners[2]);
            auto const colorBr = getTerrainColor(chunk.heightCorners[3]);

            // draw chunk triangles
            auto& vertices = grid[wOffset + w] = sf::VertexArray { sf::Triangles, 12 };

            // top triangle
            vertices[0] = { { posX, posYtl }, colorTl };
            vertices[1] = { { posX + chunkSize, posYtr }, colorTr };
            vertices[2] = { { posX + chunkSizeHalf, posYc + chunkSizeHalf }, colorC };

            // left triangle
            vertices[3] = { { posX, posYtl }, colorTl };
            vertices[4] = { { posX + chunkSizeHalf, posYc + chunkSizeHalf }, colorC };
            vertices[5] = { { posX, posYbl + chunkSize }, colorBl };

            // bot triangle
            vertices[6] = { { posX, posYbl + chunkSize }, colorBl };
            vertices[7] = { { posX + chunkSizeHalf, posYc + chunkSizeHalf }, colorC };
            vertices[8] = { { posX + chunkSize, posYbr + chunkSize }, colorBr };

            // right triangle
            vertices[9] = { { posX + chunkSize, posYbr + chunkSize }, colorBr };
            vertices[10] = { { posX + chunkSizeHalf, posYc + chunkSizeHalf }, colorC };
            vertices[11] = { { posX + chunkSize, posYtr }, colorTr };
        }
    }

    // draw terrain texture from vertices
    bool result { m->texture.create(static_cast<unsigned int>(GP::GetScreenSize().x),
        static_cast<unsigned int>(GP::GetScreenSize().y)) };
    // TODO check error
    (void)result;
    m->texture.clear(sf::Color::Black);
    // m->texture.setSmooth(true);
    for (auto const& e : grid) {
        m->texture.draw(e);
    }
    m->texture.display();

    // sprite with rendered terrain texture to display
    m->sprite = std::make_unique<jt::Sprite>();
    m->sprite->setIgnoreCamMovement(true);
    m->sprite->setPosition(jt::Vector2f(0.0f, 0.0f));
    m->sprite->fromTexture(m->texture.getTexture());
    m->sprite->update(0.0f);

    // draw terrain grid texture
    result = m->textureGrid.create(static_cast<unsigned int>(GP::GetScreenSize().x),
        static_cast<unsigned int>(GP::GetScreenSize().y));
    // TODO check error
    (void)result;
    m->textureGrid.clear(sf::Color::Transparent);
    drawTerrainGrid(m->textureGrid);
    m->textureGrid.display();

    // sprite for grid texture
    m->spriteGrid = std::make_unique<jt::Sprite>();
    m->spriteGrid->setIgnoreCamMovement(true);
    m->spriteGrid->setPosition(jt::Vector2f(0.0f, 0.0f));
    m->spriteGrid->fromTexture(m->textureGrid.getTexture());
    m->spriteGrid->update(0.0f);
}

void TerrainRenderer::doUpdate(float const elapsed) { m->sprite->update(elapsed); }

void TerrainRenderer::doDraw() const
{
    m->sprite->draw(renderTarget());
    if (m->drawGrid) {
        m->spriteGrid->draw(renderTarget());
    }
}
