#ifndef JAMTEMPLATE_RECEIVE_BUFFER_HPP
#define JAMTEMPLATE_RECEIVE_BUFFER_HPP

#include <array>
struct ReceiveBuffer {
    std::array<char, 32> size;
    std::array<char, 2048000> data;
};

#endif // JAMTEMPLATE_RECEIVE_BUFFER_HPP
