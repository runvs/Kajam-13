#include <catch2/catch.hpp>
#include <map/terrain.hpp>
#include <math_helper.hpp>
#include <vector2.hpp>

TEST_CASE("Slope is zero outside bounds", "[Terrain]")
{
    Terrain t("assets/maps/map_test.json");

    jt::Vector2f const position = GENERATE(
        // clang-format off
        jt::Vector2f{-512.0f, 0.0f}, // far left
        jt::Vector2f{(terrainWidthInChunks + 15) * terrainChunkSizeInPixel, 0}, // far right
        jt::Vector2f{ 0, -512 }, // far up
        jt::Vector2f{0, (terrainHeightInChunks + 15) * terrainChunkSizeInPixel } // far down

        // clang-format on
    );

    REQUIRE(t.getSlopeAt(position, jt::Vector2f { 1.0f, 0.0f }) == 0.0f);
    REQUIRE(t.getSlopeAt(position, jt::Vector2f { -1.0f, 0.0f }) == 0.0f);
    REQUIRE(t.getSlopeAt(position, jt::Vector2f { 0.0f, 1.0f }) == 0.0f);
    REQUIRE(t.getSlopeAt(position, jt::Vector2f { 0.0f, -1.0f }) == 0.0f);
}

TEST_CASE("Slope on steady surface is 0.0f on level zero", "[Terrain]")
{
    static Terrain t("assets/maps/map_test.json");

    auto const insideChunkOffsetX = GENERATE(range(0.0f, 15.0f, 1.0f));
    auto const insideChunkOffsetY = GENERATE(range(0.0f, 15.0f, 1.0f));

    auto const direction = GENERATE(
        // clang-format off

        jt::Vector2f { 1.0f, 0.0f },
        jt::Vector2f { -1.0f, 0.0f },
        jt::Vector2f { 0.0f, 1.0f },
        jt::Vector2f { 0.0f, -1.0f },
        jt::Vector2f { 1.0f, 1.0f },
        jt::Vector2f { 1.0f, -1.0f },
        jt::Vector2f { -1.0f, 1.0f },
        jt::Vector2f { -1.0f, -1.0f }

        // clang-format on
    );

    SECTION("test slope on level zero")
    {
        auto const chunkPosition = GENERATE(
            // clang-format off

            jt::Vector2f { 2, 2 },
            jt::Vector2f { 0, 0 }, jt::Vector2f { 1, 0 }, jt::Vector2f { 2, 0 }, jt::Vector2f { 10, 0 }, jt::Vector2f { 25, 0 },

            // around plateau of level 1, left edge
            jt::Vector2f { 1, 10 },
            jt::Vector2f { 1, 11 },
            jt::Vector2f { 1, 12 },
            jt::Vector2f { 1, 13 },
            jt::Vector2f { 1, 14 },
            jt::Vector2f { 1, 15 },
            jt::Vector2f { 1, 16 },

            // around plateau of level 1, right edge
            jt::Vector2f { 10, 10 },
            jt::Vector2f { 10, 11 },
            jt::Vector2f { 10, 12 },
            jt::Vector2f { 10, 13 },
            jt::Vector2f { 10, 14 },
            jt::Vector2f { 10, 15 },
            jt::Vector2f { 10, 16 },

            // around plateau of level 1, top edge
            jt::Vector2f { 2,  8 },
            jt::Vector2f { 3,  8 },
            jt::Vector2f { 4,  8 },
            jt::Vector2f { 5,  8 },
            jt::Vector2f { 6,  8 },
            jt::Vector2f { 7,  8 },
            jt::Vector2f { 8,  8 },
            jt::Vector2f { 9,  8 },
            jt::Vector2f { 10, 8 },

            // around plateau of level 1, bottom edge
            jt::Vector2f { 2,  17 },
            jt::Vector2f { 3,  17 },
            jt::Vector2f { 4,  17 },
            jt::Vector2f { 5,  17 },
            jt::Vector2f { 6,  17 },
            jt::Vector2f { 7,  17 },
            jt::Vector2f { 8,  17 },
            jt::Vector2f { 9,  17 },
            jt::Vector2f { 10, 17 },

            // around plateau of level 3, left edge
            jt::Vector2f { 14, 10 },
            jt::Vector2f { 14, 11 },
            jt::Vector2f { 14, 12 },
            jt::Vector2f { 14, 13 },
            jt::Vector2f { 14, 14 },
            jt::Vector2f { 14, 15 },
            jt::Vector2f { 14, 16 },


            // around plateau of level 3, right edge
            jt::Vector2f { 23, 10 },
            jt::Vector2f { 23, 11 },
            jt::Vector2f { 23, 12 },
            jt::Vector2f { 23, 13 },
            jt::Vector2f { 23, 14 },
            jt::Vector2f { 23, 15 },
            jt::Vector2f { 23, 16 },

            // around plateau of level 3, top edge
            jt::Vector2f { 15, 8 },
            jt::Vector2f { 16, 8 },
            jt::Vector2f { 17, 8 },
            jt::Vector2f { 18, 8 },
            jt::Vector2f { 19, 8 },
            jt::Vector2f { 20, 8 },
            jt::Vector2f { 21, 8 },
            jt::Vector2f { 22, 8 },
            jt::Vector2f { 23, 8 },

            // around plateau of level 3, bottom edge
            jt::Vector2f { 15, 17 },
            jt::Vector2f { 16, 17 },
            jt::Vector2f { 17, 17 },
            jt::Vector2f { 18, 17 },
            jt::Vector2f { 19, 17 },
            jt::Vector2f { 20, 17 },
            jt::Vector2f { 21, 17 },
            jt::Vector2f { 22, 17 },
            jt::Vector2f { 23, 17 }

            // clang-format on
        );

        auto const pos = chunkPosition * terrainChunkSizeInPixel
            + jt::Vector2f { insideChunkOffsetX, insideChunkOffsetY };
        REQUIRE(t.getSlopeAt(pos, direction) == 0.0f);
    }
    SECTION("test slope on level one")
    {
        auto const chunkPosition = GENERATE(
            // clang-format off

            jt::Vector2f { 4, 11 }, jt::Vector2f { 5, 11 }, jt::Vector2f { 6, 11 }, jt::Vector2f { 7, 11 }, jt::Vector2f { 8, 11 },
            jt::Vector2f { 4, 12 }, jt::Vector2f { 5, 12 }, jt::Vector2f { 6, 12 }, jt::Vector2f { 7, 12 }, jt::Vector2f { 8, 12 },
            jt::Vector2f { 4, 13 }, jt::Vector2f { 5, 13 }, jt::Vector2f { 6, 13 }, jt::Vector2f { 7, 13 }, jt::Vector2f { 8, 13 },
            jt::Vector2f { 4, 14 }, jt::Vector2f { 5, 14 }, jt::Vector2f { 6, 14 }, jt::Vector2f { 7, 14 }, jt::Vector2f { 8, 14 },
            jt::Vector2f { 4, 15 }, jt::Vector2f { 5, 15 }, jt::Vector2f { 6, 15 }, jt::Vector2f { 7, 15 }, jt::Vector2f { 8, 15 }

            // clang-format on

        );

        auto const pos = chunkPosition * terrainChunkSizeInPixel
            + jt::Vector2f { insideChunkOffsetX, insideChunkOffsetY };

        REQUIRE(t.getSlopeAt(pos, direction) == 0.0f);
    }

    SECTION("test slope on level three")
    {
        auto const chunkPosition = GENERATE(
            // clang-format off

            jt::Vector2f { 17, 11 }, jt::Vector2f { 18, 11 }, jt::Vector2f { 19, 11 }, jt::Vector2f { 20, 11 }, jt::Vector2f { 21, 11 },
            jt::Vector2f { 17, 12 }, jt::Vector2f { 18, 12 }, jt::Vector2f { 19, 12 }, jt::Vector2f { 20, 12 }, jt::Vector2f { 21, 12 },
            jt::Vector2f { 17, 13 }, jt::Vector2f { 18, 13 }, jt::Vector2f { 19, 13 }, jt::Vector2f { 20, 13 }, jt::Vector2f { 21, 13 },
            jt::Vector2f { 17, 14 }, jt::Vector2f { 18, 14 }, jt::Vector2f { 19, 14 }, jt::Vector2f { 20, 14 }, jt::Vector2f { 21, 14 },
            jt::Vector2f { 17, 15 }, jt::Vector2f { 18, 15 }, jt::Vector2f { 19, 15 }, jt::Vector2f { 20, 15 }, jt::Vector2f { 21, 15 }

            // clang-format on
        );

        auto const pos = chunkPosition * terrainChunkSizeInPixel
            + jt::Vector2f { insideChunkOffsetX, insideChunkOffsetY };

        REQUIRE(t.getSlopeAt(pos, direction) == 0.0f);
    }
}

TEST_CASE("Slope on edge of steady surface with height 1 is 0 degree or 45 degree ", "[Terrain]")
{
    static Terrain t("assets/maps/map_test.json");

    auto const edgePositionWithDirection = GENERATE(
        // clang-format off

        // left edge
        std::make_pair(jt::Vector2f { 3, 12 },jt::Vector2f { 1.0f, 0.0f }),
        std::make_pair(jt::Vector2f { 3, 13 },jt::Vector2f { 1.0f, 0.0f }),
        std::make_pair(jt::Vector2f { 3, 14 },jt::Vector2f { 1.0f, 0.0f }),

        // right edge
        std::make_pair(jt::Vector2f { 9, 12 },jt::Vector2f { -1.0f, 0.0f }),
        std::make_pair(jt::Vector2f { 9, 13 },jt::Vector2f { -1.0f, 0.0f }),
        std::make_pair(jt::Vector2f { 9, 14 },jt::Vector2f { -1.0f, 0.0f }),

        // top edge
        std::make_pair(jt::Vector2f { 5, 10 },jt::Vector2f { 0.0f, 1.0f }),
        std::make_pair(jt::Vector2f { 6, 10 },jt::Vector2f { 0.0f, 1.0f }),
        std::make_pair(jt::Vector2f { 7, 10 },jt::Vector2f { 0.0f, 1.0f }),

        // bottom edge
        std::make_pair(jt::Vector2f { 5, 16 },jt::Vector2f { 0.0f, -1.0f }),
        std::make_pair(jt::Vector2f { 6, 16 },jt::Vector2f { 0.0f, -1.0f }),
        std::make_pair(jt::Vector2f { 7, 16 },jt::Vector2f { 0.0f, -1.0f })

        // clang-format on
    );

    SECTION("Upward Slope is +45deg")
    {
        auto const slope = t.getSlopeAt(edgePositionWithDirection.first * terrainChunkSizeInPixel
                + jt::Vector2f { terrainChunkSizeInPixelHalf - 1, terrainChunkSizeInPixelHalf - 1 },
            edgePositionWithDirection.second);

        REQUIRE(slope == Approx(45.0f).epsilon(0.04));
    }
    SECTION("Downward Slope is -45deg")
    {
        auto const slope = t.getSlopeAt(edgePositionWithDirection.first * terrainChunkSizeInPixel
                + jt::Vector2f { terrainChunkSizeInPixelHalf - 1, terrainChunkSizeInPixelHalf - 1 },
            -1.0f * edgePositionWithDirection.second);

        REQUIRE(slope == Approx(-45.0f).epsilon(0.04));
    }
    SECTION("Parallel CCW is 0deg")
    {
        auto const slope = t.getSlopeAt(edgePositionWithDirection.first * terrainChunkSizeInPixel
                + jt::Vector2f { terrainChunkSizeInPixelHalf - 1, terrainChunkSizeInPixelHalf - 1 },
            jt::MathHelper::rotateBy(edgePositionWithDirection.second, 90));

        REQUIRE(std::abs(slope) <= 3.0f);
    }

    SECTION("Parallel CW is 0deg")
    {
        auto const slope = t.getSlopeAt(edgePositionWithDirection.first * terrainChunkSizeInPixel
                + jt::Vector2f { terrainChunkSizeInPixelHalf - 1, terrainChunkSizeInPixelHalf - 1 },
            jt::MathHelper::rotateBy(edgePositionWithDirection.second, -90));

        REQUIRE(std::abs(slope) <= 3.0f);
    }
}

TEST_CASE("Slope on edge of steady surface with height 3 is 0 degree or 71,5 degree ", "[Terrain]")
{
    static Terrain t("assets/maps/map_test.json");

    auto const edgePositionWithDirection = GENERATE(
        // clang-format off

        // left edge
        std::make_pair(jt::Vector2f { 16, 11 },jt::Vector2f { 1.0f, 0.0f }),
        std::make_pair(jt::Vector2f { 16, 12 },jt::Vector2f { 1.0f, 0.0f }),
        std::make_pair(jt::Vector2f { 16, 13 },jt::Vector2f { 1.0f, 0.0f }),
        std::make_pair(jt::Vector2f { 16, 14 },jt::Vector2f { 1.0f, 0.0f }),
        std::make_pair(jt::Vector2f { 16, 15 },jt::Vector2f { 1.0f, 0.0f }),

        // right edge
        std::make_pair(jt::Vector2f { 22, 11 },jt::Vector2f { -1.0f, 0.0f }),
        std::make_pair(jt::Vector2f { 22, 12 },jt::Vector2f { -1.0f, 0.0f }),
        std::make_pair(jt::Vector2f { 22, 13 },jt::Vector2f { -1.0f, 0.0f }),
        std::make_pair(jt::Vector2f { 22, 14 },jt::Vector2f { -1.0f, 0.0f }),
        std::make_pair(jt::Vector2f { 22, 15 },jt::Vector2f { -1.0f, 0.0f }),

        // top edge
        std::make_pair(jt::Vector2f { 17, 10 },jt::Vector2f { 0.0f, 1.0f }),
        std::make_pair(jt::Vector2f { 18, 10 },jt::Vector2f { 0.0f, 1.0f }),
        std::make_pair(jt::Vector2f { 19, 10 },jt::Vector2f { 0.0f, 1.0f }),
        std::make_pair(jt::Vector2f { 20, 10 },jt::Vector2f { 0.0f, 1.0f }),
        std::make_pair(jt::Vector2f { 21, 10 },jt::Vector2f { 0.0f, 1.0f }),

        // bottom edge
        std::make_pair(jt::Vector2f { 17, 16 },jt::Vector2f { 0.0f, -1.0f }),
        std::make_pair(jt::Vector2f { 18, 16 },jt::Vector2f { 0.0f, -1.0f }),
        std::make_pair(jt::Vector2f { 19, 16 },jt::Vector2f { 0.0f, -1.0f }),
        std::make_pair(jt::Vector2f { 20, 16 },jt::Vector2f { 0.0f, -1.0f }),
        std::make_pair(jt::Vector2f { 21, 16 },jt::Vector2f { 0.0f, -1.0f })

        // clang-format on
    );

    SECTION("Upward Slope is +71.5deg")
    {
        auto const slope = t.getSlopeAt(edgePositionWithDirection.first * terrainChunkSizeInPixel
                + jt::Vector2f { terrainChunkSizeInPixelHalf - 1, terrainChunkSizeInPixelHalf - 1 },
            edgePositionWithDirection.second);

        REQUIRE(slope == Approx(71.5f).epsilon(0.04));
    }
    SECTION("Downward Slope is -71.5deg")
    {
        auto const slope = t.getSlopeAt(edgePositionWithDirection.first * terrainChunkSizeInPixel
                + jt::Vector2f { terrainChunkSizeInPixelHalf - 1, terrainChunkSizeInPixelHalf - 1 },
            -1.0f * edgePositionWithDirection.second);

        REQUIRE(slope == Approx(-71.5f).epsilon(0.04));
    }
    SECTION("Parallel CCW is 0deg")
    {
        auto const slope = t.getSlopeAt(edgePositionWithDirection.first * terrainChunkSizeInPixel
                + jt::Vector2f { terrainChunkSizeInPixelHalf - 1, terrainChunkSizeInPixelHalf - 1 },
            jt::MathHelper::rotateBy(edgePositionWithDirection.second, 90));

        REQUIRE(std::abs(slope) <= 5.0f);
    }

    SECTION("Parallel CW is 0deg")
    {
        auto const slope = t.getSlopeAt(edgePositionWithDirection.first * terrainChunkSizeInPixel
                + jt::Vector2f { terrainChunkSizeInPixelHalf - 1, terrainChunkSizeInPixelHalf - 1 },
            jt::MathHelper::rotateBy(edgePositionWithDirection.second, -90));

        REQUIRE(std::abs(slope) <= 5.0f);
    }
}

TEST_CASE("Slope around hill of level 1", "[Terrain]")
{
    auto const posAndDirectionAndExpectedSlope = GENERATE(
        // clang-format off
        // on the hill, all directions
        std::make_tuple(jt::Vector2f { 7, 3 }, jt::Vector2f { 1.0f, 0.0f }, 0.0f),
        std::make_tuple(jt::Vector2f { 7, 3 }, jt::Vector2f { -1.0f, 0.0f }, 0.0f),
        std::make_tuple(jt::Vector2f { 7, 3 }, jt::Vector2f { 0.0f, 1.0f }, 0.0f),
        std::make_tuple(jt::Vector2f { 7, 3 }, jt::Vector2f { 0.0f, -1.0f }, 0.0f),

        std::make_tuple(jt::Vector2f { 7, 3 }, jt::Vector2f { 1.0f, 1.0f }, 0.0f),
        std::make_tuple(jt::Vector2f { 7, 3 }, jt::Vector2f { 1.0f, -1.0f }, 0.0f),
        std::make_tuple(jt::Vector2f { 7, 3 }, jt::Vector2f { -1.0f, 1.0f }, 0.0f),
        std::make_tuple(jt::Vector2f { 7, 3 }, jt::Vector2f { -1.0f, -1.0f }, 0.0f),

        // uphill
        // left
        std::make_tuple(jt::Vector2f { 6, 3 }, jt::Vector2f { 1.0f, 0.0f }, 45.0f),
        // right
        std::make_tuple(jt::Vector2f { 8, 3 }, jt::Vector2f { -1.0f, 0.0f }, 45.0f),
        // top
        std::make_tuple(jt::Vector2f { 7, 2 }, jt::Vector2f { 0.0f, 1.0f }, 45.0f),
        // top
        std::make_tuple(jt::Vector2f { 7, 4 }, jt::Vector2f { 0.0f, -1.0f }, 45.0f),


        // downhill
        // left
        std::make_tuple(jt::Vector2f { 6, 3 }, jt::Vector2f { -1.0f, 0.0f }, -45.0f),
        // right
        std::make_tuple(jt::Vector2f { 8, 3 }, jt::Vector2f { 1.0f, 0.0f }, -45.0f),
        // top
        std::make_tuple(jt::Vector2f { 7, 2 }, jt::Vector2f { 0.0f, -1.0f }, -45.0f),
        // top
        std::make_tuple(jt::Vector2f { 7, 4 }, jt::Vector2f { 0.0f, 1.0f }, -45.0f)

        // clang-format on
    );

    auto const insideChunkOffsetX = GENERATE(range(0.0f, 15.0f, 1.0f));
    auto const insideChunkOffsetY = GENERATE(range(0.0f, 15.0f, 1.0f));

    auto const pos = std::get<0>(posAndDirectionAndExpectedSlope) * terrainChunkSizeInPixel
        + jt::Vector2f { insideChunkOffsetX, insideChunkOffsetY };
    auto const dir = std::get<1>(posAndDirectionAndExpectedSlope);
    auto const expectedSlope = std::get<2>(posAndDirectionAndExpectedSlope);
    Terrain t("assets/maps/map_test.json");
    REQUIRE(t.getSlopeAt(pos, dir) == Approx(expectedSlope).epsilon(0.09));
}
