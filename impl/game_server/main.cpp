#include "asio.hpp"
#include "game_server.hpp"
#include <network_properties.hpp>
#include <array>
#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

int main()
{
    // TODO think about using jt::game with null window
    try {
        GameServer server;

        std::chrono::steady_clock::time_point m_timeLast {};

        for (;;) {
            // TODO proper timing
            auto const now = std::chrono::steady_clock::now();
            float const elapsedSeconds
                = std::chrono::duration_cast<std::chrono::microseconds>(now - m_timeLast).count()
                / 1000.0f / 1000.0f;
            m_timeLast = now;

            server.update(elapsedSeconds);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
