#include "vector.hpp"
#include <map/terrain.hpp>
#include <gtest/gtest.h>

TEST(TerrainSlopeOutsideBoundsTest, SlopeIsZero)
{
    Terrain t("assets/maps/map_test.json");
    ASSERT_EQ(t.getSlopeAt(jt::Vector2f { -16.0f, 0 }, jt::Vector2f { 1.0f, 0.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(jt::Vector2f { -16.0f, 0 }, jt::Vector2f { -1.0f, 0.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(jt::Vector2f { -16.0f, 0 }, jt::Vector2f { 0.0f, 1.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(jt::Vector2f { -16.0f, 0 }, jt::Vector2f { 0.0f, -1.0f }), 0.0f);

    ASSERT_EQ(
        t.getSlopeAt(jt::Vector2f { terrainWidthInChunks * 16.0f, 0 }, jt::Vector2f { 1.0f, 0.0f }),
        0.0f);
    ASSERT_EQ(t.getSlopeAt(
                  jt::Vector2f { terrainWidthInChunks * 16.0f, 0 }, jt::Vector2f { -1.0f, 0.0f }),
        0.0f);
    ASSERT_EQ(
        t.getSlopeAt(jt::Vector2f { terrainWidthInChunks * 16.0f, 0 }, jt::Vector2f { 0.0f, 1.0f }),
        0.0f);
    ASSERT_EQ(t.getSlopeAt(
                  jt::Vector2f { terrainWidthInChunks * 16.0f, 0 }, jt::Vector2f { 0.0f, -1.0f }),
        0.0f);

    ASSERT_EQ(t.getSlopeAt(
                  jt::Vector2f { 0, terrainHeightInChunks * 16.0f }, jt::Vector2f { 1.0f, 0.0f }),
        0.0f);
    ASSERT_EQ(t.getSlopeAt(
                  jt::Vector2f { 0, terrainHeightInChunks * 16.0f }, jt::Vector2f { -1.0f, 0.0f }),
        0.0f);
    ASSERT_EQ(t.getSlopeAt(
                  jt::Vector2f { 0, terrainHeightInChunks * 16.0f }, jt::Vector2f { 0.0f, 1.0f }),
        0.0f);
    ASSERT_EQ(t.getSlopeAt(
                  jt::Vector2f { 0, terrainHeightInChunks * 16.0f }, jt::Vector2f { 0.0f, -1.0f }),
        0.0f);
}

class TerrainSlopeOnSteadySurfaceOnLevelZeroParametrizedTestFixture
    : public testing::TestWithParam<jt::Vector2f> { };
TEST_P(TerrainSlopeOnSteadySurfaceOnLevelZeroParametrizedTestFixture, SlopeOnSteadySurfaceIsZero)
{
    auto pos = GetParam() * 16.0f;
    Terrain t("assets/maps/map_test.json");
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 1.0f, 0.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { -1.0f, 0.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 0.0f, 1.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 0.0f, -1.0f }), 0.0f);

    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 1.0f, 1.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 1.0f, -1.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { -1.0f, 1.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { -1.0f, -1.0f }), 0.0f);
}

INSTANTIATE_TEST_SUITE_P(TerrainSlopeOnSteadySurfaceOnLevelZeroParametrizedTest,
    TerrainSlopeOnSteadySurfaceOnLevelZeroParametrizedTestFixture,
    ::testing::Values(
        // clang-format off


jt::Vector2f { 2, 2 },
jt::Vector2f { 0, 0 }, jt::Vector2f { 1, 0 }, jt::Vector2f { 2, 0 }, jt::Vector2f { 10, 0 }, jt::Vector2f { 25, 0 },

// around plateau of level 1, left edge
jt::Vector2f { 2, 10 },
jt::Vector2f { 2, 11 },
jt::Vector2f { 2, 12 },
jt::Vector2f { 2, 13 },
jt::Vector2f { 2, 14 },
jt::Vector2f { 2, 15 },
jt::Vector2f { 2, 16 },

// around plateau of level 1, right edge
jt::Vector2f { 10, 10 },
jt::Vector2f { 10, 11 },
jt::Vector2f { 10, 12 },
jt::Vector2f { 10, 13 },
jt::Vector2f { 10, 14 },
jt::Vector2f { 10, 15 },
jt::Vector2f { 10, 16 },

// around plateau of level 1, top edge
jt::Vector2f { 2,  9 },
jt::Vector2f { 3,  9 },
jt::Vector2f { 4,  9 },
jt::Vector2f { 5,  9 },
jt::Vector2f { 6,  9 },
jt::Vector2f { 7,  9 },
jt::Vector2f { 8,  9 },
jt::Vector2f { 9,  9 },
jt::Vector2f { 10, 9 },

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
jt::Vector2f { 15, 10 },
jt::Vector2f { 15, 11 },
jt::Vector2f { 15, 12 },
jt::Vector2f { 15, 13 },
jt::Vector2f { 15, 14 },
jt::Vector2f { 15, 15 },
jt::Vector2f { 15, 16 },


// around plateau of level 3, right edge
jt::Vector2f { 23, 10 },
jt::Vector2f { 23, 11 },
jt::Vector2f { 23, 12 },
jt::Vector2f { 23, 13 },
jt::Vector2f { 23, 14 },
jt::Vector2f { 23, 15 },
jt::Vector2f { 23, 16 },

// around plateau of level 3, top edge
jt::Vector2f { 15, 9 },
jt::Vector2f { 16, 9 },
jt::Vector2f { 17, 9 },
jt::Vector2f { 18, 9 },
jt::Vector2f { 19, 9 },
jt::Vector2f { 20, 9 },
jt::Vector2f { 21, 9 },
jt::Vector2f { 22, 9 },
jt::Vector2f { 23, 9 },

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
        ));

class TerrainSlopeOnSteadySurfaceOnLevelOneParametrizedTestFixture
    : public testing::TestWithParam<jt::Vector2f> { };
TEST_P(TerrainSlopeOnSteadySurfaceOnLevelOneParametrizedTestFixture, SlopeOnSteadySurfaceIsZero)
{
    auto pos = GetParam() * 16.0f;
    Terrain t("assets/maps/map_test.json");
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 1.0f, 0.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { -1.0f, 0.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 0.0f, 1.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 0.0f, -1.0f }), 0.0f);

    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 1.0f, 1.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 1.0f, -1.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { -1.0f, 1.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { -1.0f, -1.0f }), 0.0f);
}

INSTANTIATE_TEST_SUITE_P(TerrainSlopeOnSteadySurfaceOnLevelOneParametrizedTest,
    TerrainSlopeOnSteadySurfaceOnLevelOneParametrizedTestFixture,
    ::testing::Values(
        // clang-format off

jt::Vector2f { 4, 11 }, jt::Vector2f { 5, 11 }, jt::Vector2f { 6, 11 }, jt::Vector2f { 7, 11 }, jt::Vector2f { 8, 11 },
jt::Vector2f { 4, 12 }, jt::Vector2f { 5, 12 }, jt::Vector2f { 6, 12 }, jt::Vector2f { 7, 12 }, jt::Vector2f { 8, 12 },
jt::Vector2f { 4, 13 }, jt::Vector2f { 5, 13 }, jt::Vector2f { 6, 13 }, jt::Vector2f { 7, 13 }, jt::Vector2f { 8, 13 },
jt::Vector2f { 4, 14 }, jt::Vector2f { 5, 14 }, jt::Vector2f { 6, 14 }, jt::Vector2f { 7, 14 }, jt::Vector2f { 8, 14 },
jt::Vector2f { 4, 15 }, jt::Vector2f { 5, 15 }, jt::Vector2f { 6, 15 }, jt::Vector2f { 7, 15 }, jt::Vector2f { 8, 15 }

        // clang-format on
        ));

class TerrainSlopeOnSteadySurfaceOnLevelThreeParametrizedTestFixture
    : public testing::TestWithParam<jt::Vector2f> { };
TEST_P(TerrainSlopeOnSteadySurfaceOnLevelThreeParametrizedTestFixture, SlopeOnSteadySurfaceIsZero)
{
    auto pos = GetParam() * 16.0f;
    Terrain t("assets/maps/map_test.json");
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 1.0f, 0.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { -1.0f, 0.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 0.0f, 1.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 0.0f, -1.0f }), 0.0f);

    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 1.0f, 1.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 1.0f, -1.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { -1.0f, 1.0f }), 0.0f);
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { -1.0f, -1.0f }), 0.0f);
}

INSTANTIATE_TEST_SUITE_P(TerrainSlopeOnSteadySurfaceOnLevelThreeParametrizedTestFixture,
    TerrainSlopeOnSteadySurfaceOnLevelThreeParametrizedTestFixture,
    ::testing::Values(
        // clang-format off

jt::Vector2f { 17, 11 }, jt::Vector2f { 18, 11 }, jt::Vector2f { 19, 11 }, jt::Vector2f { 20, 11 }, jt::Vector2f { 21, 11 },
jt::Vector2f { 17, 12 }, jt::Vector2f { 18, 12 }, jt::Vector2f { 19, 12 }, jt::Vector2f { 20, 12 }, jt::Vector2f { 21, 12 },
jt::Vector2f { 17, 13 }, jt::Vector2f { 18, 13 }, jt::Vector2f { 19, 13 }, jt::Vector2f { 20, 13 }, jt::Vector2f { 21, 13 },
jt::Vector2f { 17, 14 }, jt::Vector2f { 18, 14 }, jt::Vector2f { 19, 14 }, jt::Vector2f { 20, 14 }, jt::Vector2f { 21, 14 },
jt::Vector2f { 17, 15 }, jt::Vector2f { 18, 15 }, jt::Vector2f { 19, 15 }, jt::Vector2f { 20, 15 }, jt::Vector2f { 21, 15 }

        // clang-format on
        ));

class TerrainSlopeOnEdgeOnLevelOneParametrizedTestFixture
    : public testing::TestWithParam<std::tuple<jt::Vector2f, jt::Vector2f>> { };

TEST_P(TerrainSlopeOnEdgeOnLevelOneParametrizedTestFixture, UpwardSlopeIsOne)
{
    auto const pos = std::get<0>(GetParam()) * 16.0f;
    auto const dir = std::get<1>(GetParam());
    Terrain t("assets/maps/map_test.json");
    ASSERT_EQ(t.getSlopeAt(pos, dir), 1.0f);
}
TEST_P(TerrainSlopeOnEdgeOnLevelOneParametrizedTestFixture, DownwardSlopeIsMinusOne)
{
    auto const pos = std::get<0>(GetParam()) * 16.0f;
    auto const dir = -1.0f * std::get<1>(GetParam());
    Terrain t("assets/maps/map_test.json");
    ASSERT_EQ(t.getSlopeAt(pos, dir), -1.0f);
}

TEST_P(TerrainSlopeOnEdgeOnLevelOneParametrizedTestFixture, OrthogonalCWToSlopeIsZero)
{
    auto const pos = std::get<0>(GetParam()) * 16.0f;
    auto const dirParam = std::get<1>(GetParam());
    auto const dir = jt::Vector2f { dirParam.y, -dirParam.x };
    Terrain t("assets/maps/map_test.json");
    ASSERT_EQ(t.getSlopeAt(pos, dir), 0.0f);
}

TEST_P(TerrainSlopeOnEdgeOnLevelOneParametrizedTestFixture, OrthogonalCCWToSlopeIsZero)
{
    auto const pos = std::get<0>(GetParam()) * 16.0f;
    auto const dirParam = std::get<1>(GetParam());
    auto const dir = jt::Vector2f { -dirParam.y, dirParam.x };
    Terrain t("assets/maps/map_test.json");
    ASSERT_EQ(t.getSlopeAt(pos, dir), 0.0f);
}

INSTANTIATE_TEST_SUITE_P(TerrainSlopeOnEdgeOnLevelOneParametrizedTest,
    TerrainSlopeOnEdgeOnLevelOneParametrizedTestFixture,
    ::testing::Values(
        // clang-format off

// left edge
std::make_tuple(jt::Vector2f { 3, 11 },jt::Vector2f { 1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 3, 12 },jt::Vector2f { 1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 3, 13 },jt::Vector2f { 1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 3, 14 },jt::Vector2f { 1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 3, 15 },jt::Vector2f { 1.0f, 0.0f }),

// right edge
std::make_tuple(jt::Vector2f { 9, 11 },jt::Vector2f { -1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 9, 12 },jt::Vector2f { -1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 9, 13 },jt::Vector2f { -1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 9, 14 },jt::Vector2f { -1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 9, 15 },jt::Vector2f { -1.0f, 0.0f }),

// top edge
std::make_tuple(jt::Vector2f { 4, 10 },jt::Vector2f { 0.0f, 1.0f }),
std::make_tuple(jt::Vector2f { 5, 10 },jt::Vector2f { 0.0f, 1.0f }),
std::make_tuple(jt::Vector2f { 6, 10 },jt::Vector2f { 0.0f, 1.0f }),
std::make_tuple(jt::Vector2f { 7, 10 },jt::Vector2f { 0.0f, 1.0f }),
std::make_tuple(jt::Vector2f { 8, 10 },jt::Vector2f { 0.0f, 1.0f }),

// bottom edge
std::make_tuple(jt::Vector2f { 4, 16 },jt::Vector2f { 0.0f, -1.0f }),
std::make_tuple(jt::Vector2f { 5, 16 },jt::Vector2f { 0.0f, -1.0f }),
std::make_tuple(jt::Vector2f { 6, 16 },jt::Vector2f { 0.0f, -1.0f }),
std::make_tuple(jt::Vector2f { 7, 16 },jt::Vector2f { 0.0f, -1.0f }),
std::make_tuple(jt::Vector2f { 8, 16 },jt::Vector2f { 0.0f, -1.0f }),

// vertex
std::make_tuple(jt::Vector2f { 3, 10 },jt::Vector2f { 1.0f, 1.0f }),
std::make_tuple(jt::Vector2f { 3, 16 },jt::Vector2f { 1.0f, -1.0f }),
std::make_tuple(jt::Vector2f { 9, 10 },jt::Vector2f { -1.0f, 1.0f }),
std::make_tuple(jt::Vector2f { 9, 16 },jt::Vector2f { -1.0f, -1.0f })
        // clang-format on
        ));

class TerrainSlopeOnEdgeOnLevelThreeParametrizedTestFixture
    : public testing::TestWithParam<std::tuple<jt::Vector2f, jt::Vector2f>> { };

TEST_P(TerrainSlopeOnEdgeOnLevelThreeParametrizedTestFixture, UpwardSlopeIsThree)
{
    auto const pos = std::get<0>(GetParam()) * 16.0f;
    auto const dir = std::get<1>(GetParam());
    Terrain t("assets/maps/map_test.json");
    ASSERT_EQ(t.getSlopeAt(pos, dir), 3.0f);
}
TEST_P(TerrainSlopeOnEdgeOnLevelThreeParametrizedTestFixture, DownwardSlopeIsMinusThree)
{
    auto const pos = std::get<0>(GetParam()) * 16.0f;
    auto const dir = -1.0f * std::get<1>(GetParam());
    Terrain t("assets/maps/map_test.json");
    ASSERT_EQ(t.getSlopeAt(pos, dir), -3.0f);
}

TEST_P(TerrainSlopeOnEdgeOnLevelThreeParametrizedTestFixture, OrthogonalCWToSlopeIsZero)
{
    auto const pos = std::get<0>(GetParam()) * 16.0f;
    auto const dirParam = std::get<1>(GetParam());
    auto const dir = jt::Vector2f { dirParam.y, -dirParam.x };
    Terrain t("assets/maps/map_test.json");
    ASSERT_EQ(t.getSlopeAt(pos, dir), 0.0f);
}

TEST_P(TerrainSlopeOnEdgeOnLevelThreeParametrizedTestFixture, OrthogonalCCWToSlopeIsZero)
{
    auto const pos = std::get<0>(GetParam()) * 16.0f;
    auto const dirParam = std::get<1>(GetParam());
    auto const dir = jt::Vector2f { -dirParam.y, dirParam.x };
    Terrain t("assets/maps/map_test.json");
    ASSERT_EQ(t.getSlopeAt(pos, dir), 0.0f);
}

INSTANTIATE_TEST_SUITE_P(TerrainSlopeOnEdgeOnLevelThreeParametrizedTest,
    TerrainSlopeOnEdgeOnLevelThreeParametrizedTestFixture,
    ::testing::Values(
        // clang-format off

// left edge
std::make_tuple(jt::Vector2f { 16, 11 },jt::Vector2f { 1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 16, 12 },jt::Vector2f { 1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 16, 13 },jt::Vector2f { 1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 16, 14 },jt::Vector2f { 1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 16, 15 },jt::Vector2f { 1.0f, 0.0f }),

// right edge
std::make_tuple(jt::Vector2f { 22, 11 },jt::Vector2f { -1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 22, 12 },jt::Vector2f { -1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 22, 13 },jt::Vector2f { -1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 22, 14 },jt::Vector2f { -1.0f, 0.0f }),
std::make_tuple(jt::Vector2f { 22, 15 },jt::Vector2f { -1.0f, 0.0f }),

// top edge
std::make_tuple(jt::Vector2f { 17, 10 },jt::Vector2f { 0.0f, 1.0f }),
std::make_tuple(jt::Vector2f { 18, 10 },jt::Vector2f { 0.0f, 1.0f }),
std::make_tuple(jt::Vector2f { 19, 10 },jt::Vector2f { 0.0f, 1.0f }),
std::make_tuple(jt::Vector2f { 20, 10 },jt::Vector2f { 0.0f, 1.0f }),
std::make_tuple(jt::Vector2f { 21, 10 },jt::Vector2f { 0.0f, 1.0f }),

// bottom edge
std::make_tuple(jt::Vector2f { 17, 16 },jt::Vector2f { 0.0f, -1.0f }),
std::make_tuple(jt::Vector2f { 18, 16 },jt::Vector2f { 0.0f, -1.0f }),
std::make_tuple(jt::Vector2f { 19, 16 },jt::Vector2f { 0.0f, -1.0f }),
std::make_tuple(jt::Vector2f { 20, 16 },jt::Vector2f { 0.0f, -1.0f }),
std::make_tuple(jt::Vector2f { 21, 16 },jt::Vector2f { 0.0f, -1.0f }),

// vertex
std::make_tuple(jt::Vector2f { 16, 10 },jt::Vector2f { 1.0f, 1.0f }),
std::make_tuple(jt::Vector2f { 16, 16 },jt::Vector2f { 1.0f, -1.0f }),
std::make_tuple(jt::Vector2f { 22, 10 },jt::Vector2f { -1.0f, 1.0f }),
std::make_tuple(jt::Vector2f { 22, 16 },jt::Vector2f { -1.0f, -1.0f })
        // clang-format on
        ));

class TerrainSlopeOnHillOnLevelOneParametrizedTestFixture
    : public testing::TestWithParam<std::tuple<jt::Vector2f, jt::Vector2f, float>> { };

TEST_P(TerrainSlopeOnHillOnLevelOneParametrizedTestFixture, UpwardSlopeIsOne)
{
    auto const pos = std::get<0>(GetParam()) * 16.0f;
    auto const dir = std::get<1>(GetParam());
    auto const expectedSlope = std::get<2>(GetParam());
    Terrain t("assets/maps/map_test.json");
    ASSERT_EQ(t.getSlopeAt(pos, dir), expectedSlope);
}

INSTANTIATE_TEST_SUITE_P(TerrainSlopeOnHillOnLevelOneParametrizedTest,
    TerrainSlopeOnHillOnLevelOneParametrizedTestFixture,

    ::testing::Values(
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
    std::make_tuple(jt::Vector2f { 6, 3 }, jt::Vector2f { 1.0f, 0.0f }, 1.0f),
    // right
    std::make_tuple(jt::Vector2f { 8, 3 }, jt::Vector2f { -1.0f, 0.0f }, 1.0f),
    // top
    std::make_tuple(jt::Vector2f { 7, 2 }, jt::Vector2f { 0.0f, 1.0f }, 1.0f),
    // top
    std::make_tuple(jt::Vector2f { 7, 4 }, jt::Vector2f { 0.0f, -1.0f }, 1.0f),


// downhill
    // left
    std::make_tuple(jt::Vector2f { 6, 3 }, jt::Vector2f { -1.0f, 0.0f }, -1.0f),
    // right
    std::make_tuple(jt::Vector2f { 8, 3 }, jt::Vector2f { 1.0f, 0.0f }, -1.0f),
    // top
    std::make_tuple(jt::Vector2f { 7, 2 }, jt::Vector2f { 0.0f, -1.0f }, -1.0f),
    // top
    std::make_tuple(jt::Vector2f { 7, 4 }, jt::Vector2f { 0.0f, 1.0f }, -1.0f)

        // clang-format on
        ));

class TerrainSlopeOnHillOnLevelOneSubChunkParametrizedTestFixture
    : public testing::TestWithParam<jt::Vector2f> { };

TEST_P(TerrainSlopeOnHillOnLevelOneSubChunkParametrizedTestFixture, UpwardSlopeIsOne)
{
    auto const posOffset = GetParam();
    auto const pos = jt::Vector2f { 7.0f, 3.0f } * 16.0f + posOffset;

    Terrain t("assets/maps/map_test.json");
    ASSERT_EQ(t.getSlopeAt(pos, jt::Vector2f { 1.0f, 0.0f }), 0.0f);
}

INSTANTIATE_TEST_SUITE_P(TerrainSlopeOnHillOnLevelOneSubChunkParametrizedTest,
    TerrainSlopeOnHillOnLevelOneSubChunkParametrizedTestFixture,

    ::testing::Values(

        // clang-format off
jt::Vector2f{0.0f, 0.0f},

jt::Vector2f{1.0f, 0.0f},
jt::Vector2f{4.0f, 0.0f},
jt::Vector2f{8.0f, 0.0f},
jt::Vector2f{15.0f, 0.0f},

jt::Vector2f{0.0f,  1.0f},
jt::Vector2f{1.0f,  1.0f},
jt::Vector2f{4.0f,  1.0f},
jt::Vector2f{8.0f,  1.0f},
jt::Vector2f{15.0f, 1.0f},

jt::Vector2f{0.0f,  4.0f},
jt::Vector2f{1.0f,  4.0f},
jt::Vector2f{4.0f,  4.0f},
jt::Vector2f{8.0f,  4.0f},
jt::Vector2f{15.0f, 4.0f},

jt::Vector2f{0.0f,  8.0f},
jt::Vector2f{1.0f,  8.0f},
jt::Vector2f{4.0f,  8.0f},
jt::Vector2f{8.0f,  8.0f},
jt::Vector2f{15.0f, 8.0f},

jt::Vector2f{0.0f,  15.0f},
jt::Vector2f{1.0f,  15.0f},
jt::Vector2f{4.0f,  15.0f},
jt::Vector2f{8.0f,  15.0f},
jt::Vector2f{15.0f, 15.0f}

        // clang-format on
        ));
