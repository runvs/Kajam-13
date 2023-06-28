#ifndef JAMTEMPLATE_VECTOR3_HPP
#define JAMTEMPLATE_VECTOR3_HPP

#include <vector2.hpp>

namespace jt {

struct Vector3f {
    float x { 0.0f };
    float y { 0.0f };
    float z { 0.0f };

    Vector3f() = default;

    Vector3f(float x, float y, float z);
    Vector3f(jt::Vector2f const& v);
    Vector3f(jt::Vector2f const& v, float z);

    Vector3f operator+(Vector3f const& v) const;
    Vector3f operator-(Vector3f const& v) const;
    Vector3f operator*(float const v) const;

    float determinant() const;

    float length() const;
    Vector3f normalized() const;

    float dot(Vector3f const& v) const;
    Vector3f crossProduct(Vector3f const& v) const;

    float angleRad(Vector3f const& v) const;
    float angleDeg(Vector3f const& v) const;

    bool isZero() const;
};

} // namespace jt

#endif // JAMTEMPLATE_VECTOR3_HPP
