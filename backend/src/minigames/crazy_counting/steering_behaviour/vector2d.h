#ifndef VECTOR2D_H
#define VECTOR2D_H

class Vector2D {
public:
    float x, y;

    Vector2D();
    Vector2D(float x, float y);

    Vector2D operator+(const Vector2D& other) const;
    Vector2D operator-(const Vector2D& other) const;
    Vector2D operator*(float scalar) const;
    Vector2D operator/(float scalar) const;

    float length() const;
    float lengthSquared() const;
    Vector2D normalize() const;
    Vector2D truncate(float max) const;
    Vector2D clone() const;
};

#endif //VECTOR2D_H