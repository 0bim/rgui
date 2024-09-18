#pragma once
#ifndef C_BASE_ANIMATED_HPP
#define C_BASE_ANIMATED_HPP
#include "../../helpers/math/animation.hpp"

namespace rgui
{
	namespace elements
	{
		class c_base_animated
		{
		public:
			c_base_animated(math::easing::E_EASING method, const int hover_smooth, const int input_smooth)
				: m_method(method), m_hover_animation({ }), m_input_animation({ }), m_last_input_cursor_position({ }), m_hover_smooth(hover_smooth), m_input_smooth(input_smooth), m_was_hovered(false)
			{

			}

		public:
			inline math::easing::animation_t& get_input_animation()
			{
				return m_input_animation;
			}

			inline math::easing::animation_t& get_hover_animation()
			{
				return m_hover_animation;
			}

			inline float think_hover(const bool hovered)
			{
				//if it was released/just started hovering
				if (m_was_hovered != hovered)
					m_hover_animation.begin();

				//get the blend and if it isn't hovered reverse the animation (third param @animation_t::ease)
				const float blend = m_hover_animation.ease(m_method, m_hover_smooth, !hovered);

				//save hover state
				m_was_hovered = hovered;

				return blend;
			}

			inline void think_input()
			{
				if (m_input_animation.get_has_ended(m_input_smooth))
				{
					m_input_animation.begin();
					m_last_input_cursor_position = input::get_mouse_pos();
				}
			}

			inline float ease_input(const bool revered = false) const
			{
				return m_input_animation.ease(m_method, m_input_smooth, revered);
			}

			inline void draw_input_fill_animation(const shapes::rect_t& area, drawing::color_t color, float blend)
			{
				drawing::set_clip(area);

				//blend = std::fmaxf(blend, 0.1f);

				color.a = color.a - (color.a * blend); //fade down to 0
				/*const int fill_left = m_last_input_cursor_position.x - ((m_last_input_cursor_position.x - area.points.left) * ease);
				const int fill_right = ((area.points.right - m_last_input_cursor_position.x) * ease) + m_last_input_cursor_position.x;
				const int fill_top = m_last_input_cursor_position.y - ((m_last_input_cursor_position.y - area.points.top) * ease);
				const int fill_bottom = ((area.points.bottom - m_last_input_cursor_position.y) * ease) + m_last_input_cursor_position.y;
				const shapes::rect_t fill_area = { { fill_left, /*area.points.top* fill_top }, { (fill_right - fill_left), /*area.size.y* fill_bottom - fill_top } };
				drawing::draw_rectangle_filled(fill_area, color);*/

				drawing::draw_circle(m_last_input_cursor_position, std::max(area.size.x, area.size.y) * blend, color, true);

				drawing::pop_clip();
			}

			inline int get_hover_animation_smooth() const
			{
				return m_hover_smooth;
			}

			inline int get_input_animation_smooth() const
			{
				return m_input_smooth;
			}

		protected:
			math::easing::E_EASING m_method;

			math::easing::animation_t m_hover_animation;
			math::easing::animation_t m_input_animation;

			math::point_t m_last_input_cursor_position;

			int m_hover_smooth;
			int m_input_smooth;

			bool m_was_hovered;
		};
	}
}

#endif
