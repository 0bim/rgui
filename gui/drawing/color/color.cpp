#include "color.hpp"
#include <iostream>

namespace rgui
{
	namespace drawing
	{
		static int f32_to_int8_sat(float f)
		{
			return static_cast<int>(f + 0.5f);
		}

		color_t::color_t() { *this = color_t::white(); }

		color_t::color_t(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a) : r(r), g(g), b(b), a(a) { }

		color_t::color_t(const int r, const int g, const int b, const int a) : r(static_cast<std::uint8_t>(r)), g(static_cast<std::uint8_t>(g)), b(static_cast<std::uint8_t>(b)), a(static_cast<std::uint8_t>(a)) { }

		color_t::color_t(const float r, const float g, const float b, const float a) : r(static_cast<std::uint8_t>(r)), g(static_cast<std::uint8_t>(g)), b(static_cast<std::uint8_t>(b)), a(static_cast<std::uint8_t>(a)) { }

		int color_t::to_hex(const bool alpha) const
		{
			std::uint32_t out = 0;
			out = static_cast<std::uint32_t>(f32_to_int8_sat(r)) << 0;
			out |= static_cast<std::uint32_t>(f32_to_int8_sat(g)) << 8;
			out |= static_cast<std::uint32_t>(f32_to_int8_sat(b)) << 16;
			if (alpha)
			    out |= static_cast<std::uint32_t>(f32_to_int8_sat(a)) << 24;
			return out;

			//return D3DCOLOR_RGBA(static_cast<int>(r), static_cast<int>(g), static_cast<int>(b), static_cast<int>(a));
		}

		color_hsv_t color_t::to_hsv() const
		{
			float rgb[3] = { r / 255.f, g / 255.f, b / 255.f };

			color_hsv_t ret;

			float max = std::fmaxf(std::fmaxf(rgb[0], rgb[1]), rgb[2]);
			float min = std::fminf(std::fminf(rgb[0], rgb[1]), rgb[2]);

			ret.v = max;

			if (max == 0.f) 
			{
				ret.s = 0.f;
				ret.h = 0.f;
			}
			else if (max - min == 0.f)
			{
				ret.s = 0.f;
				ret.h = 0.f;
			}
			else 
			{
				ret.s = (max - min) / max;

				if (max == rgb[0])
					ret.h = 60.f * ((rgb[1] - rgb[2]) / (max - min)) + 0.f;
				else if (max == rgb[1])
					ret.h = 60.f * ((rgb[2] - rgb[0]) / (max - min)) + 120.f;
				else 
					ret.h = 60.f * ((rgb[0] - rgb[1]) / (max - min)) + 240.f;
			}

			if (ret.h < 0.f)
				ret.h += 360.f;

			return { ret.h, std::fmaxf(1.f, ret.s * 100.f), std::fmaxf(1.f, ret.v * 100.f) }; // dst_v : 0-255 // dst_h : 0-180 // dst_s : 0-255v
		}

		color_t color_t::interpolate(const color_t final_color, const float fraction) const
		{
			return { r + (final_color.r - r) * fraction, g + (final_color.g - g) * fraction, b + (final_color.b - b) * fraction, a + (final_color.a - a) * fraction };
		}

		color_t color_t::interpolate_hsv(const color_t final_color, const float fraction) const
		{
			color_t ret = to_hsv().interpolate(final_color, fraction).to_rgb();
			ret.a += (final_color.a - ret.a) * fraction;
			return ret;
		}

		color_hsv_t::color_hsv_t() { }

		color_hsv_t::color_hsv_t(float h, float s, float v) : h(std::clamp(h, 0.f, 359.f)), s(std::clamp(s, 0.f, 100.f)), v(std::clamp(v, 0.f, 100.f)) { }

		color_t color_hsv_t::to_rgb() const
		{
			//https://www.rapidtables.com/convert/color/hsv-to-rgb.html
			
			float c = (v / 100.f) * (s / 100.f);
			float x = c * (1.f - std::fabs(std::fmod(h / 60.f, 2.f) - 1.f));
			float m = (v / 100.f) - c;
			std::array<float, 3> color_arr = { 0.f, 0.f, 0.f };

			if (h >= 0.f && h < 60.f)
			{
				color_arr = { c, x, 0.f };
			}
			else if (h >= 60.f && h < 120.f)
			{
				color_arr = { x, c, 0.f };
			}
			else if (h >= 120.f && h < 180.f)
			{
				color_arr = { 0.f, c, x };
			}
			else if (h >= 180.f && h < 240.f)
			{
				color_arr = { 0.f, x, c };
			}
			else if (h >= 240.f && h < 300.f)
			{
				color_arr = { x, 0.f, c };
			}
			else if (h >= 300.f && h < 360.f)
			{
				color_arr = { c, 0.f, x };
			}
			else
			{
				color_arr = { m, m, m };
			}

			return { (color_arr[0] + m) * 255, (color_arr[1] + m) * 255, (color_arr[2] + m) * 255 };
		}

		color_hsv_t color_hsv_t::interpolate(const color_t final_color, const float fraction) const
		{
			color_hsv_t ret = *this, final_color_hsv = final_color.to_hsv();

			ret.h += (final_color_hsv.h - ret.h) * fraction;
			ret.s += (final_color_hsv.s - ret.s) * fraction;
			ret.v += (final_color_hsv.v - ret.v) * fraction;

			return ret;
		}
	}
}
