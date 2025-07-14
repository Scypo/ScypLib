#pragma once
#include <cstdint>
#include <algorithm>

namespace sl
{
    struct Color
    {
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;

        Color() = default;
        constexpr Color(float r, float g, float b, float a = 1.0f)
            : r(r), g(g), b(b), a(a) {}

        static Color FromBytes(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        {
            return
            {
                r / 255.0f,
                g / 255.0f,
                b / 255.0f,
                a / 255.0f
            };
        }

        void ToBytes(uint8_t& outR, uint8_t& outG, uint8_t& outB, uint8_t& outA) const
        {
            outR = static_cast<uint8_t>(std::clamp(r, 0.0f, 1.0f) * 255.0f);
            outG = static_cast<uint8_t>(std::clamp(g, 0.0f, 1.0f) * 255.0f);
            outB = static_cast<uint8_t>(std::clamp(b, 0.0f, 1.0f) * 255.0f);
            outA = static_cast<uint8_t>(std::clamp(a, 0.0f, 1.0f) * 255.0f);
        }

        Color operator*(float scalar) const
        {
            return { r * scalar, g * scalar, b * scalar, a * scalar };
        }

        Color operator+(const Color& other) const
        {
            return { r + other.r, g + other.g, b + other.b, a + other.a };
        }

        Color& operator*=(float scalar)
        {
            r *= scalar; g *= scalar; b *= scalar; a *= scalar;
            return *this;
        }

        Color& operator+=(const Color& other)
        {
            r += other.r; g += other.g; b += other.b; a += other.a;
            return *this;
        }

        static Color Lerp(const Color& a, const Color& b, float t)
        {
            return
            {
                a.r + (b.r - a.r) * t,
                a.g + (b.g - a.g) * t,
                a.b + (b.b - a.b) * t,
                a.a + (b.a - a.a) * t
            };
        }
    };

    namespace Colors
    {
        static constexpr Color White = Color(1.0f, 1.0f, 1.0f);
        static constexpr Color Black = Color(0.0f, 0.0f, 0.0f);
        static constexpr Color Gray = Color(0.502f, 0.502f, 0.502f);
        static constexpr Color LightGray = Color(0.827f, 0.827f, 0.827f);
        static constexpr Color Red = Color(1.0f, 0.0f, 0.0f);
        static constexpr Color Green = Color(0.0f, 1.0f, 0.0f);
        static constexpr Color Blue = Color(0.0f, 0.0f, 1.0f);
        static constexpr Color Yellow = Color(1.0f, 1.0f, 0.0f);
        static constexpr Color Cyan = Color(0.0f, 1.0f, 1.0f);
        static constexpr Color Magenta = Color(1.0f, 0.0f, 1.0f);
        static constexpr Color Orange = Color(1.0f, 0.647f, 0.0f);
    }
}