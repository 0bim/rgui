#pragma once
#ifndef POINT_T_HPP
#define POINT_T_HPP
#include <cmath>
#include <limits>
#include <ostream>

namespace rgui
{
	namespace math
	{
		struct point_t
		{
			point_t()
			{
				this->x = this->y = std::numeric_limits<int>::infinity();
			}

			template<typename T>
			point_t(const T x, const T y)
			{
				this->x = x;
				this->y = y;
			}

			template<typename T, typename V>
			point_t(const T x, const V y)
			{
				this->x = x;
				this->y = y;
			}

			inline static point_t zero()
			{
				return { 0, 0 };
			}

			inline point_t operator+(const point_t& other) const
			{
				return { x + other.x, y + other.y };
			}

			inline point_t operator+(const int number) const
			{
				return { x + number, y + number };
			}

			inline point_t operator+=(const point_t& other)
			{
				return *this = *this + other;
			}

			inline point_t operator-(const point_t& other) const
			{
				return { x - other.x, y - other.y };
			}

			inline point_t operator-(const int number) const
			{
				return { x - number, y - number };
			}

			inline point_t operator-=(const point_t& other) 
			{
				return *this = *this - other;
			}

			inline point_t operator*(const point_t& other) const
			{
				return { x * other.x, y * other.y };
			}

			inline point_t operator*=(const point_t& other)
			{
				return *this = *this * other;
			}

			inline point_t operator*(const int& number) const
			{
				return { x * number, y * number };
			}

			inline point_t operator*(const float& number) const
			{
				return { x * number, y * number };
			}

			inline point_t operator*=(const int& number)
			{
				return *this = *this * number;
			}

			inline point_t operator*=(const float& number)
			{
				return *this = *this * number;
			}

			inline bool operator<(const point_t& other) const
			{
				return x < other.x && y < other.y;
			}

			inline bool operator>(const point_t& other) const
			{
				return x > other.x&& y > other.y;
			}

			inline bool operator==(const point_t& other) const
			{
				return x == other.x && y == other.y;
			}

			inline bool operator!=(const point_t& other) const
			{
				return !(*this == other);
			}

			inline point_t operator/(const int number) const
			{
				return { x / number, y / number };
			}

			inline point_t abs() const
			{
				return { std::abs(x), std::abs(y) };
			}

			inline int length() const
			{
				return std::sqrt(std::pow(x, 2) + std::pow(y, 2));
			}

			inline int dist_to(const point_t& other) const
			{
				return std::sqrt(std::pow(x - other.x, 2) + std::pow(y - other.y, 2));
			}

			inline bool is_valid() const
			{
				return true;
			}

			/*inline point_t scale_dpi() const
			{
				// Determine the current screen DPI.
				const HDC hDC = GetDC(0);
				const int dpi_x = GetDeviceCaps(hDC, LOGPIXELSX);
				const int dpi_y = GetDeviceCaps(hDC, LOGPIXELSY);
				ReleaseDC(0, hDC);

				return { MulDiv(x, dpi_x, 96), MulDiv(y, dpi_y, 96) };
			}*/

			inline point_t scale_ascpect_ratio(const float scale) const
			{
				const math::point_t no_ratio = { x * scale, y * scale };
				//original height / original width * new width = new height
				//orignal width / orignal height * new height = new width
				//lawlz
				return no_ratio;
				//return { y == 0 ? 0 : static_cast<float>(x) / static_cast<float>(y) * static_cast<float>(no_ratio.y), x == 0 ? 0 : static_cast<float>(y) / static_cast<float>(x) * static_cast<float>(no_ratio.x) };
			}

			int x, y;
		};

#ifdef _DEBUG
		inline std::ostream& operator<<(std::ostream& stream, const point_t& point)
		{
			return stream << ("(") << point.x << (", ") << point.y << (")");
		}
#endif
	}
}
#endif