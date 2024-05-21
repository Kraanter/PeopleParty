#include "vector2d.h"
#include <cmath>

Vector2D::Vector2D() : Vector2D(0, 0) {}

Vector2D::Vector2D(float x, float y) {
    this->x = x;
    this->y = y;
}

Vector2D Vector2D::operator+(const Vector2D& other) const {
    return Vector2D(x + other.x, y + other.y);
}

Vector2D Vector2D::operator-(const Vector2D& other) const {
    return Vector2D(x - other.x, y - other.y);
}

Vector2D Vector2D::operator*(float scalar) const {
    return Vector2D(x * scalar, y * scalar);
}

Vector2D Vector2D::operator/(float scalar) const {
    return Vector2D(x / scalar, y / scalar);
}


float Vector2D::length() const {
    return std::sqrt(x * x + y * y);
}

float Vector2D::lengthSquared() const {
    return x * x + y * y;
}

Vector2D Vector2D::normalize() const {
    float len = length();
    if (len == 0) return Vector2D(0, 0);
    return Vector2D(x / len, y / len);
}

Vector2D Vector2D::truncate(float max) const {
    if (length() > max) {
        return normalize() * max;
    }
    return *this;
}

Vector2D Vector2D::clone() const {
    return Vector2D(x, y);
}