#include "compression/compressor_impl.hpp"
#include "log/logger_null.hpp"
#include "network_data/unit_client_to_server_data.hpp"
#include "network_data/unit_info_collection.hpp"
#include "random/random.hpp"
#include "receive_buffer.hpp"
#include "simulation_result_sender_interface.hpp"
#include "system_helper.hpp"
#include <game_simulation.hpp>
#include <gtest/gtest.h>

class NetworkMessageCollector : public SimulationResultSenderInterface {
public:
    std::string messageString { "" };
    void sendSimulationResults(SimulationResultDataForAllFrames const& data) override
    {
        nlohmann::json j = data;
        messageString = j.dump();
    }
};

TEST(NetworkDataSizeTest, NoUnitsSize)
{
    jt::null_objects::LoggerNull logger {};
    UnitInfoCollection units { logger, std::vector<UnitInfo> {} };
    GameSimulation gs { logger, units };
    gs.prepareSimulationForNewRound();

    NetworkMessageCollector collector;
    gs.performSimulation(collector);

    CompressorImpl compressor;
    auto const compressed = compressor.compress(collector.messageString);
    std::cout << "Network data size for simulation with no units: " << compressed.size()
              << std::endl;

    // ensure the data fits into the receive buffer.
    ReceiveBuffer buffer;
    EXPECT_LE(compressed.size(), buffer.data.size());
    //    std::cout << collector.messageString << std::endl;
}

TEST(NetworkDataSizeTest, OneHundretArchers)
{
    jt::null_objects::LoggerNull logger {};
    UnitInfoCollection units { logger, "assets/units/all_units.json" };
    GameSimulation gs { logger, units };
    for (auto i = 0; i != 100; ++i) {
        UnitClientToServerData data;
        data.unitType = "archer";
        data.playerID = jt::Random::getInt(0, 1);
        data.unitID = i;
        data.positionX = (i / 5) * 32;
        data.positionY = (i % 5) * 32;
        gs.addUnit(data);
    }

    gs.prepareSimulationForNewRound();

    NetworkMessageCollector collector;
    gs.performSimulation(collector);

    CompressorImpl compressor;
    auto const compressed = compressor.compress(collector.messageString);
    std::cout << "Network data size for simulation with 100 archers: " << compressed.size()
              << std::endl;

    // ensure the data fits into the receive buffer.
    ReceiveBuffer buffer;
    EXPECT_LE(compressed.size(), buffer.data.size());
}

TEST(NetworkDataSizeTest, TwoHundretRandomUnits)
{
    jt::null_objects::LoggerNull logger {};
    UnitInfoCollection units { logger, "assets/units/all_units.json" };
    GameSimulation gs { logger, units };
    auto const types = units.getTypes();
    for (auto i = 0; i != 200; ++i) {
        UnitClientToServerData data;
        data.unitType = *jt::SystemHelper::select_randomly(types.cbegin(), types.cend());
        data.playerID = jt::Random::getInt(0, 1);
        data.unitID = i;
        data.positionX = (i / 5) * 32;
        data.positionY = (i % 5) * 32;
        gs.addUnit(data);
    }

    gs.prepareSimulationForNewRound();

    NetworkMessageCollector collector;
    gs.performSimulation(collector);

    CompressorImpl compressor;
    auto const compressed = compressor.compress(collector.messageString);
    std::cout << "Network data size for simulation with 200 random units: " << compressed.size()
              << std::endl;

    // ensure the data fits into the receive buffer.
    ReceiveBuffer buffer;
    EXPECT_LE(compressed.size(), buffer.data.size());
}
