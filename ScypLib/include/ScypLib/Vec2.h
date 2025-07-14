#pragma once
#include <cmath>
#include <functional>

namespace sl
{
    template<typename T>
    class Vec2
    {
    public:
        Vec2() = default;
        Vec2(T x_in, T y_in)
            : x(x_in), y(y_in) {}

        Vec2(const Vec2& other) = default;
        Vec2(Vec2&& other) noexcept = default;

        Vec2& operator=(const Vec2& rhs)
        {
            if (this != &rhs)
            {
                x = rhs.x;
                y = rhs.y;
            }
            return *this;
        }

        Vec2& operator=(Vec2&& rhs) noexcept
        {
            if (this != &rhs)
            {
                x = std::move(rhs.x);
                y = std::move(rhs.y);
            }
            return *this;
        }

        bool operator==(const Vec2& other) const
        {
            return x == other.x && y == other.y;
        }

        bool operator!=(const Vec2& other) const
        {
            return !(*this == other);
        }

        Vec2 operator+(const Vec2& rhs) const
        {
            return Vec2(x + rhs.x, y + rhs.y);
        }

        Vec2& operator+=(const Vec2& rhs)
        {
            *this = *this + rhs;
            return *this;
        }

        Vec2 operator-(const Vec2& rhs) const
        {
            return Vec2(x - rhs.x, y - rhs.y);
        }

        Vec2& operator-=(const Vec2& rhs)
        {
            *this = *this - rhs;
            return *this;
        }

        Vec2 operator*(T rhs) const
        {
            return Vec2(x * rhs, y * rhs);
        }

        Vec2& operator*=(T rhs)
        {
            *this = *this * rhs;
            return *this;
        }

        Vec2 operator/(T rhs) const
        {
            return Vec2(x / rhs, y / rhs);
        }

        Vec2& operator/=(T rhs)
        {
            *this = *this / rhs;
            return *this;
        }

        T GetLength() const
        {
            return (T)std::sqrt(GetLengthSq());
        }

        T GetLengthSq() const
        {
            return x * x + y * y;
        }

        Vec2& Normalize()
        {
            return *this = this->GetNormalized();
        }

        Vec2 GetNormalized() const
        {
            const T len = GetLength();
            if (len != (T)0)
                return *this * ((T)1 / len);
            return *this;
        }

        T Dot(const Vec2& other) const
        {
            return x * other.x + y * other.y;
        }

        Vec2 Perpendicular() const
        {
            return Vec2(-y, x);
        }

        bool IsZero() const
        {
            return x == (T)0 && y == (T)0;
        }

        template<typename U>
        explicit Vec2(const Vec2<U>& other)
            : x(static_cast<T>(other.x)), y(static_cast<T>(other.y))
        {}

        template<typename U>
        explicit operator Vec2<U>() const
        {
            return Vec2<U>(static_cast<U>(x), static_cast<U>(y));
        }

    public:
        T x;
        T y;
    };
    using Vec2f = Vec2<float>;
    using Vec2i = Vec2<int>;
}

namespace std 
{
    template <typename T>
    struct hash<sl::Vec2<T>> 
    {
        std::size_t operator()(const sl::Vec2<T>& vec) const 
        {
            std::size_t hx = std::hash<T>()(vec.x);
            std::size_t hy = std::hash<T>()(vec.y);
            return hx ^ (hy << 1);
        }
    };
}
