#pragma once
#include <array>
#include <map>
#include <algorithm>
#include <d3d9.h>

namespace rgui
{
	namespace drawing
	{
        struct color_hsv_t;

        struct color_argb_t
        {
            color_argb_t()
                :a(255), r(255), g(255), b(255)
            {

            }

            color_argb_t(const int a, const int r, const int g, const int b)
                : a(a), r(r), g(g), b(b)
            {

            }

            std::uint8_t a, r, g, b;
        };

        struct color_t
        {
            color_t();
            color_t(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a = 255);
            color_t(const int r, const int g, const int b, const int a = 255);
            color_t(const float r, const float g, const float b, const float a = 255.f);

            int to_hex(const bool alpha = true) const;

            color_hsv_t to_hsv() const;

            color_t interpolate(const color_t final_color, const float fraction) const;

            color_t interpolate_hsv(const color_t final_color, const float fraction) const;

            inline bool operator==(const color_t& other)
            {
                return r == other.r && g == other.g && b == other.b && a == other.a;
            }

            inline void operator/=(const std::uint8_t number)
            {
                r /= number, g /= number, b /= number, a /= number;
            }

            inline color_t operator/(const std::uint8_t number) const
            {
                return { r / number, g / number, b / number, a / number };
            }

            inline float* to_float_arr() const
            {
                std::array<float, 4> arr = { static_cast<float>(r), static_cast<float>(g), static_cast<float>(b), static_cast<float>(a) };
                return arr.data();
            }

            inline unsigned char* to_uchar_arr() const
            {
                std::array<unsigned char, 4> arr = { static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(a) };
                return arr.data();
            }

            inline char* to_char_arr() const
            {
                std::array<char, 4> arr = { static_cast<char>(r), static_cast<char>(g), static_cast<char>(b), static_cast<char>(a) };
                return arr.data();
            }

            inline static color_t red() { return { 255, 0, 0 }; }
            inline static color_t orange() { return { 255, 128, 0 }; }
            inline static color_t yellow() { return { 255, 255, 0 }; }
            inline static color_t green() { return { 0, 255, 0 }; }
            inline static color_t dark_green() { return { 0, 100, 0 }; }
            inline static color_t purple() { return { 150, 0, 255 }; }
            inline static color_t pink() { return { 255, 105, 180 }; }
            inline static color_t blue() { return { 0, 0, 255 }; }
            inline static color_t cyan() { return { 0, 255, 255 }; }
            inline static color_t white() { return { 255, 255, 255 }; }
            inline static color_t black() { return { 0, 0, 0 }; }
            inline static color_t grey() { return { 210, 210, 210 }; };

            std::uint8_t r, g, b, a;
		};

        struct color_t;

        struct color_hsv_t
        {
            color_hsv_t();
            color_hsv_t(float h, float s = 100.f, float v = 100.f);

            color_t to_rgb() const;

            color_hsv_t interpolate(const color_t final_color, const float fraction) const;

            float h, s, v;
        };
	}
}
