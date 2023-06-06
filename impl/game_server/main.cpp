#include "compression/compressor_impl.hpp"
#include "compression/logging_compressor.hpp"
#include "log/default_logging.hpp"
#include "log/logger.hpp"
#include <game_server.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

int main()
{
    jt::Logger logger;
    jt::createDefaultLogTargets(logger);

    auto compressor = std::make_shared<CompressorImpl>();
    LoggingCompressor loggingCompressor { logger, compressor };
    // TODO think about using jt::game with null window
    try {
        GameServer server { logger, loggingCompressor };

        std::chrono::steady_clock::time_point m_timeLast {};

        for (;;) {
            auto const now = std::chrono::steady_clock::now();
            float const elapsedSeconds
                = std::chrono::duration_cast<std::chrono::microseconds>(now - m_timeLast).count()
                / 1000.0f / 1000.0f;
            m_timeLast = now;

            server.update(elapsedSeconds);
            std::this_thread::sleep_for(std::chrono::milliseconds { 16 });
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
