#pragma once
#include <cmath>
#include <functional>
#include"Vec2.h"

namespace sl
{
    template<typename T>
    class Vec3
    {
    public:
        Vec3() = default;
        Vec3(T x, T y, T z)
            : x(x), y(y), z(z) {}

        Vec3(const Vec3& other) = default;
        Vec3(Vec3&& other) noexcept = default;
        Vec3(const Vec2<T>& vec2, T z)
            : x(vec2.x), y(vec2.y), z(z) {}

        Vec3& operator=(const Vec3& rhs)
        {
            if (this != &rhs)
            {
                x = rhs.x;
                y = rhs.y;
                z = rhs.z;
            }
            return *this;
        }

        Vec3& operator=(Vec3&& rhs) noexcept
        {
            if (this != &rhs)
            {
                x = std::move(rhs.x);
                y = std::move(rhs.y);
                z = std::move(rhs.z);
            }
            return *this;
        }

        bool operator==(const Vec3& other) const
        {
            return x == other.x && y == other.y && z == other.z;
        }

        bool operator!=(const Vec3& other) const
        {
            return !(*this == other);
        }

        Vec3 operator+(const Vec3& rhs) const
        {
            return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
        }

        Vec3& operator+=(const Vec3& rhs)
        {
            *this = *this + rhs;
            return *this;
        }

        Vec3 operator-(const Vec3& rhs) const
        {
            return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
        }

        Vec3& operator-=(const Vec3& rhs)
        {
            *this = *this - rhs;
            return *this;
        }

        Vec3 operator*(T rhs) const
        {
            return Vec3(x * rhs, y * rhs, z * rhs);
        }

        Vec3& operator*=(T rhs)
        {
            *this = *this * rhs;
            return *this;
        }

        Vec3 operator/(T rhs) const
        {
            return Vec3(x / rhs, y / rhs, z / rhs);
        }

        Vec3& operator/=(T rhs)
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
            return x * x + y * y + z * z;
        }

        Vec3& Normalize()
        {
            return *this = this->GetNormalized();
        }

        Vec3 GetNormalized() const
        {
            const T len = GetLength();
            if (len != (T)0)
                return *this * ((T)1 / len);
            return *this;
        }

        T Dot(const Vec3& other) const
        {
            return x * other.x + y * other.y + z * other.z;
        }

        Vec3 Cross(const Vec3& other) const
        {
            return Vec3(
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            );
        }

        template<typename U>
        explicit Vec3(const Vec3<U>& other)
            : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)), z(static_cast<T>(other.z))
        {}

        template<typename U>
        explicit operator Vec3<U>() const
        {
            return Vec3<U>(static_cast<U>(x), static_cast<U>(y), static_cast<U>(z));
        }

    public:
        T x{};
        T y{};
        T z{};
    };

    using Vec3f = Vec3<float>;
    using Vec3i = Vec3<int>;
}

namespace std
{
    template <typename T>
    struct hash<sl::Vec3<T>>
    {
        std::size_t operator()(const sl::Vec3<T>& v) const noexcept
        {
            const std::size_t hx = std::hash<T>{}(v.x);
            const std::size_t hy = std::hash<T>{}(v.y);
            const std::size_t hz = std::hash<T>{}(v.z);

            std::size_t seed = hx;
            seed ^= hy + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hz + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    };
}
