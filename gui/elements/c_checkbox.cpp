#include "c_checkbox.hpp"

namespace rgui
{
	namespace elements
	{
		c_checkbox::c_checkbox(const std::string& title, const math::point_t& position, const math::point_t& size)
			: c_base_element(title, position, size), c_base_animated(math::easing::E_EASING::IN_OUT_SINE, 400, 300), c_base_variable_controller(), c_base_value_change_callback()
		{
			m_rounding = 2;
		}

		void c_checkbox::draw()
		{
			const auto& area = get_geometry_area();

			const bool is_enabled = get_value(), is_hovered = input::get_is_mouse_in_area({ m_position, { m_size.x + m_style->get_font(E_STYLE_FONT::SMALL)->get_text_size(get_title_formatted()).x + 5, m_size.y } });

			drawing::color_t accent = m_style->get_color(E_STYLE_COLOR::ACCENT);
			const float alpha = accent.a;
			accent.a = alpha * ease_input(!is_enabled);
			drawing::draw_rectangle_filled(area, accent, m_rounding);

			if (is_enabled)
			{
				static drawing::color_t checkmark_color = drawing::color_t::black();
				checkmark_color.a = accent.a;

				draw_checkmark(area.position, area.size, checkmark_color);


				//drawing::draw_line({ area.points.left + 2, area.points.top + m_size.y / 2 - 1 }, { area.points.left + m_size.x / 2, area.points.top + m_size.y - 2 }, checkmark_color, 2);
				//drawing::draw_line({ area.points.left + m_size.x / 2, area.points.top + m_size.y - 2 }, { area.points.left + m_size.x - 2, area.points.top + m_size.y / 2 - 4 }, checkmark_color, 2);
			}
			else
				drawing::draw_rectangle(area, m_style->get_color(E_STYLE_COLOR::GENERAL_OUTLINE), m_rounding);

			accent.a = alpha * think_hover(is_hovered);
			drawing::draw_rectangle(area, accent, m_rounding);

			m_style->get_font(E_STYLE_FONT::SMALL)->draw({ area.position.x + area.size.x + 5, area.position.y + area.size.y / 2 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL), get_title_formatted(), drawing::TEXTFLAG_CENTERED_Y);
		}

		bool c_checkbox::process_input(const input::input_capture_t& input, const float scroll)
		{
			const auto& area = get_geometry_area();
			if (input::get_is_mouse_in_area({ area.position, { area.size.x + m_style->get_font(E_STYLE_FONT::SMALL)->get_text_size(get_title_formatted()).x + 5, area.size.y } }))
			{
				if (input.key == VK_LBUTTON && input.get_key_state().is_pressed())
				{
					*m_variable ^= true;
					think_input();
					run_value_change_callback(get_value());

					return true;
				}
			}

			return false;
		}

		void c_checkbox::update()
		{

		}
	}
}