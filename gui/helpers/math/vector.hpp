#pragma once
#include <cmath>
#include <limits>

#ifndef VECTOR_HPP
#define VECTOR_HPP
#undef max

namespace rgui
{
	namespace math
	{
		struct point_t;

		struct vector2d
		{
			vector2d(const point_t& point);

			vector2d()
			{
				invalidate();
			}

			vector2d(const float x, const float y)
			{
				this->x = x, this->y = y;
			}

			vector2d(const int x, const int y)
			{
				this->x = x, this->y = y;
			}

			inline bool operator>(const vector2d other_vec) const
			{
				return this->x > other_vec.x && this->y > other_vec.y;
			}

			inline bool operator>=(vector2d other_vec)
			{
				return this->x >= other_vec.x && this->y >= other_vec.y;
			}

			inline bool operator<(const vector2d other_vec) const
			{
				return this->x < other_vec.x && this->y < other_vec.y;
			}

			inline bool operator<=(const vector2d other_vec) const
			{
				return this->x <= other_vec.x && this->y <= other_vec.y;
			}

			inline bool operator==(const vector2d other_vec) const
			{
				return this->x == other_vec.x && this->y == other_vec.y;
			}

			inline bool operator==(const float num) const
			{
				return this->x == num && this->y == num;
			}

			inline bool operator!=(const vector2d other_vec) const
			{
				return this->x != other_vec.x && this->y != other_vec.y;
			}

			/*inline void operator=(const point_t point)
			{
				this->x = point.x;
				this->y = point.y;
			}*/

			inline void operator=(const float num)
			{
				this->x = num;
				this->y = num;
			}

			inline vector2d operator+(const float num) const
			{
				return { this->x + num, this->y + num };
			}

			inline vector2d operator+(const vector2d other_vec) const
			{
				return { this->x + other_vec.x, this->y + other_vec.y };
			}

			inline vector2d operator-(const float num) const
			{
				return { this->x - num, this->y - num };
			}

			inline vector2d operator-(const vector2d other_vec) const
			{
				return { this->x - other_vec.x, this->y - other_vec.y };
			}

			inline vector2d operator+=(const vector2d other_vec)
			{
				if (!is_valid())
					*this = zero();

				*this = *this + other_vec;
				return *this;
			}

			inline vector2d operator-=(const vector2d other_vec)
			{
				if (!is_valid())
					*this = zero();

				*this = *this - other_vec;
				return *this;
			}

			inline vector2d operator/(const float num) const
			{
				return { this->x / num, this->y / num };
			}

			inline vector2d operator*(const float num) const
			{
				return { this->x * num, this->y * num };
			}

			inline vector2d operator*=(const float num)
			{
				*this = { x * num, y * num };
			}

			inline vector2d abs() const
			{
				return { std::abs(this->x), std::abs(this->y) };
			}

			inline float dist_to(vector2d other_vec) const
			{
				return std::abs(std::sqrtf(std::powf(this->x - other_vec.x, 2) + std::powf(this->y - other_vec.y, 2))); //d = sqrt( (x - x2)^2 + (y - y2)^2)
			}

			inline void invalidate()
			{
				*this = std::numeric_limits<float>::infinity();
			}

			inline bool is_valid() const
			{
				return !(*this == std::numeric_limits<float>::infinity());
			}

			inline static vector2d zero()
			{
				return { 0.f, 0.f };
			}

			inline bool is_zero() const
			{
				return *this == zero();
			}

			inline static vector2d max(const vector2d a, const vector2d b)
			{
				return a > b ? a : b;
			}

			inline static vector2d max_coords(const vector2d a, const vector2d b)
			{
				return { std::fmaxf(a.x, b.x), std::fmaxf(a.y, b.y) };
			}

			float dot(const vector2d& o) const
			{
				return (x * o.x) + (y * o.y);
			}

			float dot() const
			{
				return dot(*this);
			}

			float length_sqr() const
			{
				return dot();
			}

			float length() const
			{
				return std::sqrt(length_sqr());
			}

			float reciprocal_length() const
			{
				return 1.f / length();
			}

			vector2d normalized() const
			{
				vector2d ret = *this;
				return (ret *= reciprocal_length());
			}

			void normalize()
			{
			    *this = normalized();
			}

			float x;
			float y;
		};

		struct vector3d
		{
			float x, y, z;
		};
	}
}
#endif