#ifndef JAMTEMPLATE_TERRAIN_RENDERER_HPP
#define JAMTEMPLATE_TERRAIN_RENDERER_HPP

#include <game_object.hpp>
#include <map/terrain.hpp>
#include <atomic>
#include <memory>

class TerrainRenderer : public jt::GameObject {
    struct Private;
    std::shared_ptr<Private> m;
    Terrain const& m_terrain;
    std::atomic_bool m_dirty {};

public:
    TerrainRenderer(Terrain const& t);

    void setDrawGrid(bool v);

private:
    void updateFromTerrain();
    void doCreate() override;
    void doUpdate(float const elapsed) override;
    void doDraw() const override;
};

#endif // JAMTEMPLATE_TERRAIN_RENDERER_HPP
