#ifndef VECTOR2D_H
#define VECTOR2D_H

struct Vector2D
{
    float x;
    float y;
    Vector2D();
    Vector2D(float a, float b);
    Vector2D(const Vector2D& v);
    inline float Length()const;
    inline float LengthSq()const;
    inline float Distance(const Vector2D &v2)const;
    inline float DistanceSq(const Vector2D &v2)const;
    const Vector2D& Normalize();
    const Vector2D& Truncate(float max);
    const Vector2D operator+(const Vector2D &rhs);
    const Vector2D operator-(const Vector2D &rhs);
    const Vector2D operator*(const float &rhs);
    const Vector2D operator/(const float &rhs);
    const Vector2D& operator+=(const Vector2D &rhs);
    const Vector2D& operator-=(const Vector2D &rhs);
    const Vector2D& operator*=(const float& rhs);
    const Vector2D& operator/=(const float& rhs);
    bool operator==(const Vector2D& rhs)const;
    bool operator!=(const Vector2D& rhs)const;
};

#endif //VECTOR2D_H