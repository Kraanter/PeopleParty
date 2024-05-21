#include "vector2d.h"
#include <cmath>

Vector2D::Vector2D() : Vector2D(0, 0) {}

Vector2D::Vector2D(float x, float y) {
    this->x = x;
    this->y = y;
}

Vector2D::Vector2D(const Vector2D &v) {
    x = v.x;
    y = v.y;
}

float Vector2D::Length() const {
    return sqrt(x * x + y * y);
}

float Vector2D::LengthSq() const {
    return x * x + y * y;
}

float Vector2D::Distance(const Vector2D &v2) const {
    float ySeparation = v2.y - y;
    float xSeparation = v2.x - x;

    return sqrt(ySeparation * ySeparation + xSeparation * xSeparation);
}

float Vector2D::DistanceSq(const Vector2D &v2) const {
    float ySeparation = v2.y - y;
    float xSeparation = v2.x - x;

    return ySeparation * ySeparation + xSeparation * xSeparation;
}

const Vector2D &Vector2D::Normalize() {
    float length = Length();
    if (length > 0) {
        x /= length;
        y /= length;
    }

    return *this;
}

const Vector2D &Vector2D::Truncate(float max) {
    if (Length() > max) {
        Normalize();
        *this *= max;
    }

    return *this;
}


const Vector2D &Vector2D::operator+(const Vector2D &rhs) {
    return Vector2D(x + rhs.x, y + rhs.y);
}

const Vector2D &Vector2D::operator-(const Vector2D &rhs) {
    return Vector2D(x - rhs.x, y - rhs.y);
}

const Vector2D &Vector2D::operator*(const float &rhs) {
    return Vector2D(x * rhs, y * rhs);
}

const Vector2D &Vector2D::operator/(const float &rhs) {
    return Vector2D(x / rhs, y / rhs);
}

const Vector2D &Vector2D::operator+=(const Vector2D &rhs) {
    x += rhs.x;
    y += rhs.y;

    return *this;
}

const Vector2D &Vector2D::operator-=(const Vector2D &rhs) {
    x -= rhs.x;
    y -= rhs.y;

    return *this;
}

const Vector2D &Vector2D::operator*=(const float &rhs) {
    x *= rhs;
    y *= rhs;

    return *this;
}

const Vector2D &Vector2D::operator/=(const float &rhs) {
    x /= rhs;
    y /= rhs;

    return *this;
}

bool Vector2D::operator==(const Vector2D &rhs) const {
    return x == rhs.x && y == rhs.y;
}

bool Vector2D::operator!=(const Vector2D &rhs) const {
    return x != rhs.x || y != rhs.y;
}