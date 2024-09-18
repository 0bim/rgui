#pragma once
#ifndef C_SLIDER_HPP
#define C_SLIDER_HPP
#include <limits>
#include <string_view>
#include "base/c_base_element.hpp"
#include "base/c_base_callback.hpp"
#include "base/c_base_draggable.hpp"
#include "base/c_base_variable_controller.hpp"

namespace rgui
{
	namespace elements
	{
		template<typename T>
		class c_slider : public c_base_element, public c_base_callback, public c_base_draggable, public c_base_variable_controller<T>
		{
		public:
			c_slider(const std::string& title, const math::point_t & position, const math::point_t & size = { 235, 8 })
				: c_base_element(title, position, size), c_base_callback(), c_base_draggable(), c_base_variable_controller<T>(), m_marker_radius(8), m_min(std::numeric_limits<T>::min()), m_max(std::numeric_limits<T>::max()), m_difference(std::numeric_limits<T>::min()), m_old_fill(0), m_display_value(true), m_fill_animation({})
			{

			}

		public:
			void draw() override final
			{
				auto small_font = m_style->get_font(E_STYLE_FONT::SMALL);
				const auto& area = get_geometry_area();
				drawing::draw_rectangle_filled(area, m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND), 10);

				const auto draw_slider_bar = [&](const int fill, drawing::color_t color)
				{
					const int delta = fill - area.position.x;
					if (delta > 0)
					    drawing::draw_rectangle_filled({ area.position, { delta, area.size.y } }, color, 10);

					if (m_marker_radius > 0)
					{
						const int ball_fill = fill < m_position.x + m_marker_radius ? fill + m_marker_radius : fill < m_position.x + m_marker_radius ? fill - m_marker_radius : fill;
						const math::point_t ball_position = { ball_fill, static_cast<int>(area.position.y + m_marker_radius * m_scale * 0.5f) };
						if (get_is_being_dragged())
							drawing::draw_circle(ball_position, m_marker_radius * 1.45f, color, true);

						color.a *= 2;
						drawing::draw_circle(ball_position, m_marker_radius, color, true);
					}
				};

				drawing::color_t fill_color = m_style->get_color(E_STYLE_COLOR::ACCENT);
				const std::uint8_t original_alpha = fill_color.a;

				if (input::get_is_mouse_in_area(get_geometry_area()) && m_fill_animation.get_has_ended(100))
				{
					fill_color.a = original_alpha * 0.3f;
					draw_slider_bar(get_fill_width(get_value_slide(input::get_mouse_pos().x)), fill_color);
				}

				const float blend = m_fill_animation.ease(math::easing::E_EASING::OUT_SINE, 250);
				fill_color.a = original_alpha * 0.5f;

				draw_slider_bar(m_old_fill + (get_fill_width(this->get_value()) - m_old_fill) * blend, fill_color);

				small_font->draw({ area.position.x, area.position.y - 8 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_HIGHLIGHTED),get_title_formatted(), drawing::TEXTFLAG_BOTTOM);

				if (m_display_value)
				{
					char buffer[256]; //buffer overflow possible with too many digits
					//std::sprintf(buffer, (std::is_same_v<T, float> ? (std::fmod(this->get_value(), 1) /*is not whole number*/ ? "%.2f" : "%.0f") : "%d"), this->get_value());
					std::sprintf(buffer, std::is_same_v<T, float> ? "%.2f" : "%d", this->get_value());
					small_font->draw({ area.position.x + area.size.x, area.position.y - 8 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_HIGHLIGHTED), std::string(buffer), drawing::TEXTFLAG_BOTTOM | drawing::TEXTFLAG_LEFT);
				}
			}

			bool process_input(const input::input_capture_t& input, const float scroll) override final
			{
				if (input.key == VK_LBUTTON)
				{
					const auto& area = get_geometry_area();

					const bool hovered = input::get_is_mouse_in_area({ { area.position.x - m_marker_radius, area.position.y - m_marker_radius  }, { area.size.x + m_marker_radius * 2, area.size.y + m_marker_radius * 2 } });
					if (input.get_key_state().is_pressed())
					{
						//add an extra pixel on each side for ease of use
						if (hovered)
						{
							m_old_fill = get_fill_width(this->get_value());
							this->set_value(get_value_slide(input::get_mouse_pos().x));
							m_fill_animation.begin();
							run_input_callback();
							return true;
						}
					}
					else
					{
						if (m_is_being_dragged)
						{
							m_old_fill = get_fill_width(this->get_value());
							this->set_value(get_value_slide(input::get_mouse_pos().x));

							if (!get_is_target_element() || input.get_key_state().is_idle()) //released
							{
								set_is_being_dragged(false);
								run_input_callback();
								return false; //make sure to return false so it doesnt set it to the target again
							}

							return true;
						}
						else 
						{
							if (hovered && input.get_key_state().is_held())
							{
								if (m_fill_animation.get_has_ended(250))
								    set_is_being_dragged(true);
								return true;
							}
						}
					}

					set_is_being_dragged(false);
				}

				return false;
			}

			void update() override final
			{

			}

		public:
			inline void on_retarget() override final
			{
				set_is_being_dragged(false);
			}

			inline void set_limits(const T min, const T max)
			{
				m_min = min;
				m_max = max;
			}

			inline T get_min() const
			{
				return m_min;
			}

			inline T get_max() const
			{
				return m_max;
			}

			inline void set_value(T value) override final
			{
				c_base_variable_controller<T>::set_value(std::clamp(value, std::max<T>(m_min, std::numeric_limits<T>::min()), std::min<T>(std::numeric_limits<T>::max(), m_max)));
			}

			inline T get_value_slide(const int slide)
			{
				const float delta = slide - get_position().x;
				float normalized_addon_val = ((delta / get_size().x) * (m_max - m_min) + m_min);
				if (!std::is_same_v<float, T> && !std::is_same_v<double, T>)
				{
					if ((m_max - m_min) * 0.5f >= normalized_addon_val)
						normalized_addon_val = std::floor(normalized_addon_val);
					else
						normalized_addon_val = normalized_addon_val + 1;
				}

				//N = 647
				//650

				if (m_difference > std::numeric_limits<T>::min())
					normalized_addon_val += static_cast<float>(m_difference) - std::fmodf(normalized_addon_val, static_cast<float>(m_difference));

				T clamped_addon_val = std::clamp(normalized_addon_val, static_cast<float>(std::max<T>(m_min, std::numeric_limits<T>::min())), static_cast<float>(std::min<T>(std::numeric_limits<T>::max(), m_max)));

				return clamped_addon_val;
			}

			inline int get_fill_width(const T value)
			{
				int fill_width = get_position().x + get_size().x * (value - m_min) / (m_max - m_min);
				fill_width = fill_width < 0 ? get_position().x : (fill_width > (get_position().x + get_size().x) ? (get_position().x + get_size().x) : fill_width);
				return fill_width;
			}

			inline void set_marker_radius(const int radius)
			{
				m_marker_radius = radius;
			}

			inline int get_marker_radius() const
			{
				return m_marker_radius;
			}

			inline void set_display_value(const bool flag)
			{
				m_display_value = flag;
			}

			inline bool get_display_value() const
			{
				return m_display_value;
			}

			inline void set_port_area(const math::point_t& area) override final
			{
				c_base_element::set_port_area(area);
				set_size({ area.x, m_size.y });
			}

			inline void set_difference(const T difference)
			{
				m_difference = difference;
			}

			inline T get_difference() const
			{
				return m_difference;
			}

		private:
			int m_marker_radius;

			T m_min, m_max;

			T m_difference;

			int m_old_fill;

			bool m_display_value;

			math::easing::animation_t m_fill_animation;
		};
	}
}

#endif