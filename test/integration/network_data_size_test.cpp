#include <catch2/catch.hpp>
#include <compression/compressor_impl.hpp>
#include <game_simulation.hpp>
#include <log/logger_null.hpp>
#include <map/terrain.hpp>
#include <network_data/unit_client_to_server_data.hpp>
#include <network_data/unit_info_collection.hpp>
#include <random/random.hpp>
#include <receive_buffer.hpp>
#include <simulation_result_sender_interface.hpp>
#include <system_helper.hpp>
#include <iostream>

class NetworkMessageCollector : public SimulationResultSenderInterface {
public:
    std::string messageString { "" };

    void sendSimulationResults(SimulationResultDataForAllFrames const& data) override
    {
        nlohmann::json j = data;
        messageString = j.dump();
    }
};

TEST_CASE("Size of game simulation network data", "[Integration, NetworkDataSizeTest]")
{
    jt::null_objects::LoggerNull logger {};
    UnitInfoCollection units { logger, "assets/units/all_units.json" };
    GameSimulation gs { logger, units };
    NetworkMessageCollector collector;
    SECTION("No units") { }
    SECTION("100 Archers")
    {
        for (auto i = 0; i != 100; ++i) {
            UnitClientToServerData data;
            data.unitType = "archer";
            data.playerID = jt::Random::getInt(0, 1);
            data.unitID = i;
            data.positionX = (i % terrainWidthInChunks) * terrainChunkSizeInPixel;
            data.positionY = (i / terrainWidthInChunks) * terrainChunkSizeInPixel;
            gs.addUnit(data);
        }
    }
    SECTION("200 random units")
    {
        auto const types = units.getTypes();
        for (auto i = 0; i != 200; ++i) {
            UnitClientToServerData data;
            data.unitType = *jt::SystemHelper::select_randomly(types.cbegin(), types.cend());
            data.playerID = jt::Random::getInt(0, 1);
            data.unitID = i;
            data.positionX = (i % terrainWidthInChunks) * terrainChunkSizeInPixel;
            data.positionY = (i / terrainWidthInChunks) * terrainChunkSizeInPixel;
            gs.addUnit(data);
        }
    }
    gs.prepareSimulationForNewRound();
    gs.performSimulation(collector);

    CompressorImpl compressor;
    auto const compressed = compressor.compress(collector.messageString);
    std::cout << "Network data size for simulation with no units: " << compressed.size()
              << std::endl;

    // ensure the data fits into the receive buffer.
    ReceiveBuffer buffer;
    REQUIRE(compressed.size() < buffer.data.size());
}
