#pragma once

#ifndef SHAPES_HPP
#define SHAPES_HPP

#include <array>
#include <DirectXMath.h>
#include "../math/math.hpp"
#include "../math/point_t.hpp"

namespace rgui
{
	namespace shapes
	{
		struct points_t
		{
			points_t() 
			: left(0), top(0), right(0), bottom(0) 
			{ 

			}

			template<typename T>
			points_t(const T left, const T top, const T right, const T bottom)
				: left(left), top(top), right(right), bottom(bottom)
			{

			}

			int left, top, right, bottom;
		};

		struct shape_t
		{
		public:
			shape_t() { }

			shape_t(const math::point_t& position)
				: position(position)
			{

			}

			math::point_t position;
		};

		struct rect_t : public shape_t
		{
			rect_t(const math::point_t& position = { }, const math::point_t& size = { })
				: shape_t(position)
			{
				this->size = size;

				update_points();
			}

			template<typename T>
			rect_t(const T left, const T top, const T right, const T bottom)
			{
				this->position = { left, top };
				this->size = { right - left, bottom - top };

				points = { left, top, right, bottom };
			}

			inline bool operator==(const rect_t& other) const
			{
				return (points.left == other.points.top &&
					points.right == other.points.right &&
					points.bottom == other.points.bottom &&
					points.top == other.points.top);
			}

			inline bool intersect(const rect_t& other) const
			{
				return ((points.left <= other.points.right && points.left >= other.points.left) || (points.top <= other.points.bottom && points.top >= other.points.top)) || (((points.left <= other.points.left && points.right >= other.points.left) && (points.bottom >= other.points.top && points.top <= other.points.bottom || points.top <= other.points.top && points.bottom >= other.points.top)));
			}

			inline void update_points()
			{
				points.top = position.y; //top left = rect first point
				points.left = position.x;
				points.bottom = position.y + size.y;
				points.right = position.x + size.x;
			}

			inline bool is_valid() const
			{
				return position.is_valid() && size.is_valid(); /*(&&
					std::isfinite(points.left) &&
					std::isfinite(points.right) &&
					std::isfinite(points.bottom) &&
					std::isfinite(points.top);*/
			}

			inline rect_t scale_ascpect_ratio(const float scale = 1.f) const
			{
				return { position.scale_ascpect_ratio(scale), size.scale_ascpect_ratio(scale) };
			}

			math::point_t size;
			points_t points;
		};

		struct circle_t : public shape_t
		{
		public:
			circle_t(const math::point_t& pos, float radius) : shape_t(pos)
			{
				this->radius = radius;
			}

			float radius;
		};

		struct triangle_t : public shape_t
		{
		public:
			triangle_t(const math::point_t& pos, const std::array<math::point_t, 3>& points) : shape_t(pos)
			{
				this->points = points;
			}

			std::array<math::point_t, 3> points;
		};


		
	}
}

#endif