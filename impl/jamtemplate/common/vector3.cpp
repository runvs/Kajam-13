#include "vector3.hpp"
#include <math_helper.hpp>
#include <cmath>

jt::Vector3f::Vector3f(float x, float y, float z)
    : x { x }
    , y { y }
    , z { z }
{
}

jt::Vector3f::Vector3f(jt::Vector2f const& v, float z)
    : x { v.x }
    , y { v.y }
    , z { z }
{
}

jt::Vector3f jt::Vector3f::operator+(Vector3f const& v) const
{
    return { x + v.x, y + v.y, z + v.z };
}

jt::Vector3f jt::Vector3f::operator-(Vector3f const& v) const
{
    return { x - v.x, y - v.y, z - v.z };
}

jt::Vector3f jt::Vector3f::operator*(float const v) const { return { x * v, y * v, z * v }; }

float jt::Vector3f::determinant() const { return x * x + y * y + z * z; }

float jt::Vector3f::length() const { return std::sqrt(determinant()); }

jt::Vector3f jt::Vector3f::normalized() const
{
    auto const len = length();
    if (len != 0.0f) {
        return { x / len, y / len, z / len };
    }
    return {};
}

float jt::Vector3f::dot(Vector3f const& v) const { return x * v.x + y * v.y + z * v.z; }

jt::Vector3f jt::Vector3f::crossProduct(Vector3f const& v) const
{
    return { y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x };
}

float jt::Vector3f::angleRad(Vector3f const& v) const
{
    return acosf(dot(v) / std::sqrt(determinant() * v.determinant()));
}

float jt::Vector3f::angleDeg(Vector3f const& v) const
{
    return static_cast<float>(jt::MathHelper::rad2deg(angleRad(v)));
}

bool jt::Vector3f::isZero() const { return x == 0 && y == 0 && z == 0; }
