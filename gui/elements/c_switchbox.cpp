#include "c_switchbox.hpp"

namespace rgui
{
	namespace elements
	{
		c_switchbox::c_switchbox(const std::string& title, const math::point_t& position, const math::point_t& size)
			: c_base_element(title, position, size), c_base_animated(math::easing::E_EASING::IN_OUT_SINE, 500, 250), c_base_variable_controller(), c_base_value_change_callback()
		{

		}

		void c_switchbox::draw()
		{
			const auto& area = get_geometry_area();

			const float blend = ease_input(!get_value());

			m_style->get_font(E_STYLE_FONT::SMALL)->draw({ area.position.x, static_cast<int>(area.position.y + area.size.y * 0.5f) }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL).interpolate(m_style->get_color(E_STYLE_COLOR::ELEMENT_HIGHLIGHTED), blend),get_title_formatted(), drawing::TEXTFLAG_CENTERED_Y);

			drawing::draw_rectangle_filled({ { area.position.x + get_port_area().x - area.size.x, area.position.y }, area.size }, m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND), 10);

			const float ball_radius = area.size.y * 0.65f;
			 
			const shapes::rect_t adjusted_area = { { area.position.x + get_port_area().x - area.size.x, area.position.y }, { static_cast<int>(std::fmaxf(ball_radius * 1.5f, area.size.x * blend)), area.size.y } };

			drawing::color_t ball_fill_color = m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL).interpolate(m_style->get_color(E_STYLE_COLOR::ACCENT), blend);

			const std::uint8_t alpha = m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL).a;
			ball_fill_color.a = alpha * 0.5f;
			drawing::draw_rectangle_filled(adjusted_area, ball_fill_color, 10);
			//drawing::draw_rectangle(adjusted_area, m_style->get_color(input::get_is_mouse_in_area(adjusted_area) ? E_STYLE_COLOR::ELEMENT_HIGHLIGHTED : E_STYLE_COLOR::ELEMENT_GENERAL), 10);

			ball_fill_color.a = alpha;
			drawing::draw_circle({ adjusted_area.position.x + (ball_radius * (1.f - blend)) + ((area.size.x - ball_radius + 1) * blend), adjusted_area.position.y + area.size.y * 0.5f }, ball_radius, ball_fill_color, true);
		}

		bool c_switchbox::process_input(const input::input_capture_t& input, const float scroll)
		{
			if (input.key == VK_LBUTTON && input.get_key_state().is_pressed())
			{
				const auto& area = get_geometry_area();
				if (input::get_is_mouse_in_area({ { area.position.x + get_port_area().x - area.size.x, area.position.y }, area.size }) || input::get_is_mouse_in_area({ area.position, m_style->get_font(E_STYLE_FONT::SMALL)->get_text_size(get_title_formatted()) }))
				{
					if (m_variable && m_input_animation.get_has_ended(m_input_smooth))
					{
						*m_variable ^= true;

						think_input();
						run_value_change_callback(get_value());

						return true;
					}
				}
			}

			return false;
		}

		void c_switchbox::update()
		{

		}
	}
}