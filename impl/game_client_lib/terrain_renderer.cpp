#include <game_properties.hpp>
#include <math_helper.hpp>
#include <random/random.hpp>
#include <sprite.hpp>
#include <terrain_renderer.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <cmath>

namespace {

constexpr float const chunkSize { terrainChunkSizeInPixel };
constexpr auto const chunkSizeHalf = chunkSize / 2.0f;

sf::Color const colorStone { 100, 100, 100 };

sf::Color const colorMap[] {
    // clang-format off
    // dirt layer
    { 89, 60, 47 },
    // grassy layer
    { 53, 99, 50 }, { 41, 79, 39 },
    // hill layer
    { 80, 80, 80 }, { 120, 120, 120 },
    // mountain layer
    { 160, 160, 160 }, { 200, 200, 200 }
    // clang-format on
};

float getTerrainHeight(int y, float height)
{
    return y * chunkSize - height * terrainHeightScalingFactor;
}

sf::Color getTerrainColor(float height)
{
    height = jt::MathHelper::clamp(height, 0.0f, terrainHeightMax);
    // dirty
    if (height < 0.4) {
        return colorMap[0];
    }
    // grassy
    if (height < 1.4) {
        return colorMap[1];
    }
    if (height < 2.4) {
        return colorMap[2];
    }
    // hilly
    if (height < 3.7) {
        return colorMap[3];
    }
    if (height < 4.2) {
        return colorMap[4];
    }
    // mountainy
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
    std::unique_ptr<sf::RenderTexture> texture {};
    std::unique_ptr<jt::Sprite> sprite {};
    sf::RenderTexture textureGrid {};
    std::unique_ptr<jt::Sprite> spriteGrid {};
};

TerrainRenderer::TerrainRenderer(Terrain const& t)
    : m { std::make_shared<TerrainRenderer::Private>() }
    , m_terrain { t }
{
    m_terrain.setOnUpdate([this]() { m_dirty = true; });
}

void TerrainRenderer::setDrawGrid(bool v) { m->drawGrid = v; }

void TerrainRenderer::doCreate()
{
    // sprite with rendered terrain texture to display
    m->sprite = std::make_unique<jt::Sprite>();
    m->sprite->setIgnoreCamMovement(true);
    m->sprite->setPosition(jt::Vector2f(0.0f, 0.0f));

    // draw terrain grid texture
    (void)m->textureGrid.create(static_cast<unsigned int>(GP::GetScreenSize().x),
        static_cast<unsigned int>(GP::GetScreenSize().y));
    m->textureGrid.clear(sf::Color::Transparent);
    drawTerrainGrid(m->textureGrid);
    m->textureGrid.display();

    // sprite for grid texture
    m->spriteGrid = std::make_unique<jt::Sprite>();
    m->spriteGrid->setIgnoreCamMovement(true);
    m->spriteGrid->setPosition(jt::Vector2f(0.0f, 0.0f));
    m->spriteGrid->fromTexture(m->textureGrid.getTexture());
    m->spriteGrid->update(0.0f);

    updateFromTerrain();
}

void TerrainRenderer::updateFromTerrain()
{
    // create chunk to vertex map
    std::array<sf::VertexArray, terrainWidthInChunks * terrainHeightInChunks> grid;
    std::array<sf::VertexArray, terrainWidthInChunks * terrainHeightInChunks> gridDecals;
    auto const& chunks = m_terrain.getChunks();
    for (unsigned short h { 0 }; h < terrainHeightInChunks; ++h) {
        bool const lastLine { h == terrainHeightInChunks - 1 };
        auto const wOffset = h * terrainWidthInChunks;
        for (unsigned short w { 0 }; w < terrainWidthInChunks; ++w) {
            auto const chunkIdx = wOffset + w;
            auto const& chunk = chunks[chunkIdx];
            auto const posX = w * chunkSize;
            auto const posY = h * chunkSize;
            jt::Rectf const chunkRect { posX, posY, chunkSize, chunkSize };
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
            auto& vertices = grid[chunkIdx] = sf::VertexArray { sf::Triangles, 12u };

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

            // draw stones for dirt layer
            if (chunk.heightCenter < 0.1
                || (chunk.heightCenter >= 2.8 && chunk.heightCenter <= 3.7)) {
                auto const numStones
                    = static_cast<std::size_t>(jt::Random::getChance(0.25f) ? 1 : 0);
                auto& decals = gridDecals[chunkIdx]
                    = sf::VertexArray { sf::TriangleFan, numStones * 6 };
                for (auto i = 0u; i != numStones; ++i) {
                    auto const pos = jt::Random::getRandomPointIn(chunkRect);
                    decals[i * 6 + 0] = { { pos.x, pos.y }, colorStone };
                    decals[i * 6 + 1] = { { pos.x + 3, pos.y - 1 }, colorStone };
                    decals[i * 6 + 2] = { { pos.x + 2, pos.y - 2 }, colorStone };
                    decals[i * 6 + 3] = { { pos.x, pos.y - 3 }, colorStone };
                    decals[i * 6 + 4] = { { pos.x - 2, pos.y - 2 }, colorStone };
                    decals[i * 6 + 5] = { { pos.x - 3, pos.y - 1 }, colorStone };
                }
            }
            // draw grass decals for grass
            else if (chunk.heightCenter >= 0.3 && chunk.heightCenter <= 1.6) {
                jt::Rectf const chunkRect { posX, posY, chunkSize, chunkSize };
                auto const numGrass = static_cast<std::size_t>(jt::Random::getInt(1, 3));
                auto& decals = gridDecals[chunkIdx] = sf::VertexArray { sf::Lines, numGrass * 6 };
                for (auto i = 0u; i != numGrass; ++i) {
                    auto const pos = jt::Random::getRandomPointIn(chunkRect);
                    auto const colOffset
                        = jt::Random::getInt(4, 9) * (jt::Random::getChance(0.5f) ? -1 : 1);
                    sf::Color const col { static_cast<sf::Uint8>(colorC.r + colOffset),
                        static_cast<sf::Uint8>(colorC.g + colOffset),
                        static_cast<sf::Uint8>(colorC.b + colOffset) };
                    decals[i * 6 + 0] = { { pos.x, pos.y }, col };
                    decals[i * 6 + 1] = { { pos.x - 1, pos.y - 4 }, col };
                    decals[i * 6 + 2] = { { pos.x - 1, pos.y }, col };
                    decals[i * 6 + 3] = { { pos.x - 3, pos.y - 3 }, col };
                    decals[i * 6 + 4] = { { pos.x + 1, pos.y }, col };
                    decals[i * 6 + 5] = { { pos.x + 2, pos.y - 3 }, col };
                }
            } else {
                gridDecals[chunkIdx] = sf::VertexArray { sf::Points, 0 };
            }
        }
    }

    auto texture = std::make_unique<sf::RenderTexture>();

    // draw terrain texture from vertices
    (void)texture->create(static_cast<unsigned int>(GP::GetScreenSize().x),
        static_cast<unsigned int>(GP::GetScreenSize().y));
    texture->clear(sf::Color::Black);
    for (auto const& e : grid) {
        texture->draw(e);
    }
    for (auto const& e : gridDecals) {
        texture->draw(e);
    }
    texture->display();

    m->sprite->fromTexture(texture->getTexture());
    m->sprite->update(0.0f);

    m->texture = std::move(texture);
}

void TerrainRenderer::doUpdate(float const elapsed)
{
    if (m_dirty) {
        m_dirty = false;
        updateFromTerrain();
    }
    m->sprite->update(elapsed);
}

void TerrainRenderer::doDraw() const
{
    m->sprite->draw(renderTarget());
    if (m->drawGrid) {
        m->spriteGrid->draw(renderTarget());
    }
}
