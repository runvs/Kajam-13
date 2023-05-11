#ifndef JAMTEMPLATE_RECT_HPP
#define JAMTEMPLATE_RECT_HPP

#include <object_properties.hpp>
#include <vector.hpp>
#include <map>
#include <string>

namespace jt {

namespace tilemap {

struct InfoRect {
    jt::Vector2f position { 0.0f, 0.0f };
    jt::Vector2f size { 0.0f, 0.0f };
    float rotation { 0.0f };
    std::string type { "" };
    std::string name { "" };
    ObjectProperties properties {};
};

} // namespace tilemap
} // namespace jt
#endif
