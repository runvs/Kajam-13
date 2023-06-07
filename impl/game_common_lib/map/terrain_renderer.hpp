#pragma once

#include "terrain.hpp"
#include <game_object.hpp>
#include <memory>

class TerrainRenderer : public jt::GameObject {
    struct Private;
    std::shared_ptr<Private> m;
    Terrain const& m_terrain;

public:
    TerrainRenderer(Terrain const& t);

private:
    void doCreate() override;

    void doUpdate(float const elapsed) override;

    void doDraw() const override;
};
