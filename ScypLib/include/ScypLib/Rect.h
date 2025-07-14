#pragma once
#include "Vec2.h"

namespace sl
{
    template<typename T>
    class Rect
    {
    public:
        Rect(T left, T right, T top, T bottom)
            : left(left), right(right), top(top), bottom(bottom) {}

        Rect(const Vec2<T>& topLeft, const Vec2<T>& bottomRight)
            : Rect(topLeft.x, bottomRight.x, topLeft.y, bottomRight.y) {}

        Rect(const Vec2<T>& topLeft, T width, T height)
            : Rect(topLeft, topLeft + Vec2<T>(width, height)) {}

        bool IsOverlappingWith(const Rect& other) const
        {
            return right > other.left && left < other.right &&
                bottom > other.top && top < other.bottom;
        }

        bool IsContainedBy(const Rect& other) const
        {
            return left >= other.left && right <= other.right &&
                top >= other.top && bottom <= other.bottom;
        }

        bool Contains(const Vec2<T>& point) const
        {
            return point.x >= left && point.x < right && point.y >= top && point.y < bottom;
        }

        Rect FromCenter(const Vec2<T>& center, T halfWidth, T halfHeight) const
        {
            const Vec2<T> half(halfWidth, halfHeight);
            return Rect(center - half, center + half);
        }

        Rect GetExpanded(T offset) const
        {
            return Rect(left - offset, right + offset, top - offset, bottom + offset);
        }

        Vec2<T> GetCenter() const
        {
            return Vec2<T>((left + right) / (T)2, (top + bottom) / (T)2);
        }

        T GetWidth() const { return right - left; }
        T GetHeight() const { return bottom - top; }

        bool operator==(const Rect& other) const
        {
            return left == other.left && right == other.right &&
                top == other.top && bottom == other.bottom;
        }

        bool operator!=(const Rect& other) const
        {
            return !(*this == other);
        }

        Rect operator*(T scalar) const
        {
            return Rect(left * scalar, right * scalar, top * scalar, bottom * scalar);
        }

        Rect operator/(T scalar) const
        {
            return Rect(left / scalar, right / scalar, top / scalar, bottom / scalar);
        }

        Rect operator*(const Vec2<T>& scale) const
        {
            return Rect(
                Vec2<T>(left * scale.x, top * scale.y),
                Vec2<T>(right * scale.x, bottom * scale.y)
            );
        }

        Rect operator/(const Vec2<T>& scale) const
        {
            return Rect(
                Vec2<T>(left / scale.x, top / scale.y),
                Vec2<T>(right / scale.x, bottom / scale.y)
            );
        }
        template<typename U>
        explicit Rect(const Rect<U>& other)
            : left(static_cast<T>(other.left)),
            right(static_cast<T>(other.right)),
            top(static_cast<T>(other.top)),
            bottom(static_cast<T>(other.bottom))
        {}
        
        template<typename U>
        explicit operator Rect<U>() const
        {
            return Rect<U>(static_cast<U>(left), static_cast<U>(right), static_cast<U>(top), static_cast<U>(bottom));
        }

        T left, right, top, bottom;
    };
    using RectI = Rect<int>;
    using RectF = Rect<float>;
}
namespace std 
{
    template <typename T>
    struct hash<sl::Rect<T>> 
    {
        std::size_t operator()(const sl::Rect<T>& rect) const 
        {
            std::size_t seed = 0;

            auto hash_combine = [&seed](std::size_t value) 
                {
                    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                };

            hash_combine(std::hash<T>{}(rect.left));
            hash_combine(std::hash<T>{}(rect.right));
            hash_combine(std::hash<T>{}(rect.top));
            hash_combine(std::hash<T>{}(rect.bottom));

            return seed;
        }
    };
}