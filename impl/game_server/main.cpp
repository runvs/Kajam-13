#include "asio.hpp"
#include <network_properties.hpp>
#include <array>
#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

std::string make_daytime_string()
{
    using namespace std; // For time_t, time and ctime;
    time_t now = time(nullptr);
    return ctime(&now);
}

int main()
{
    try {
        asio::io_context io_context;

        asio::ip::udp::socket socket(io_context,
            asio::ip::udp::endpoint(
                NetworkProperties::NetworkProtocolType(), NetworkProperties::DefaultServerPort()));

        std::cout << "start listening for connections\n";
        for (;;) {
            std::array<char, 1024> recv_buf;
            asio::ip::udp::endpoint remote_endpoint;
            std::cout << "wait for message\n";
            auto const length = socket.receive_from(asio::buffer(recv_buf), remote_endpoint);

            std::cout << "received message from endpoint: ' " << remote_endpoint.address() << " : "
                      << remote_endpoint.port() << " '\n";

            std::stringstream ss;
            ss.write(recv_buf.data(), length);
            auto const str = ss.str();
            std::cout << str << "\n";

            std::string message = make_daytime_string();

            asio::error_code ignored_error;
            std::cout << "send message to endpoint: ' " << remote_endpoint.address() << " : "
                      << remote_endpoint.port() << " '\n";
            socket.send_to(asio::buffer(message), remote_endpoint, 0, ignored_error);
            std::cout << ignored_error.message() << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
