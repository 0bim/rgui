#include "c_button.hpp"

namespace rgui
{
	namespace elements
	{
		c_button::c_button(const std::string& title, const math::point_t& position, const math::point_t& size)
			: c_base_element(title, position, size), c_base_animated(math::easing::E_EASING::IN_OUT_SINE, 600, 200), c_base_icon(), c_base_callback(), m_animate(true), m_outlined(true), m_is_background_dependant(true), m_dynamic_size(false), m_has_plus(true)
		{
			m_rounding = 3;
			push_flag(E_ELEMENT_FLAG::NO_TARGET);
		}

		void c_button::draw()
		{
			const auto& area = get_geometry_area();
			const float blend_hover = think_hover(input::get_is_mouse_in_area(area));

			const bool is_enabled = get_is_enabled();
			const bool is_hovered = input::get_is_mouse_in_area(area);

			if (get_is_background_dependant())
			    drawing::draw_rectangle_filled(area, m_style->get_color(is_enabled ? E_STYLE_COLOR::ACCENT : E_STYLE_COLOR::ELEMENT_BACKGROUND), m_rounding);

			if (m_animate)
		 	    draw_input_fill_animation(area, m_style->get_color(E_STYLE_COLOR::ACCENT), ease_input());

			//drawing::color_t outline_color = m_style->get_color(E_STYLE_COLOR::ACCENT);
			//outline_color.a *= blend_hover;

			if (get_has_plus())
			{
				const float square_size = area.size.y * 0.38f;
				drawing::draw_rectangle_filled({ { area.points.left + 10, area.points.top + area.size.y * 0.31f }, { square_size, square_size } }, m_style->get_color(is_enabled || is_hovered ? E_STYLE_COLOR::ACCENT : E_STYLE_COLOR::ELEMENT_GENERAL), m_rounding);
				drawing::draw_rectangle_filled({ { area.points.left + 10 + square_size * 0.5f, area.points.top + area.size.y * 0.31f + 1 }, { 1, square_size - 2 } }, m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND));
				drawing::draw_rectangle_filled({ { area.points.left + 11, area.points.top + area.size.y * 0.31f + square_size * 0.5f - 1}, { square_size - 2, 1 } }, m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND));
			}

			//if (m_outlined)
			    //drawing::draw_rectangle(area, m_style->get_color(E_STYLE_COLOR::GENERAL_OUTLINE), m_rounding);

			//if (m_animate)
			    //drawing::draw_rectangle_filled(area, outline_color, m_rounding);

			if (m_texture->is_valid())
			{
				m_texture->set_size(area.size);
				drawing::draw_image(area.position, m_texture, area.size.x * 0.5f, drawing::color_t(255, 255, 255, static_cast<int>(m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL).a)));
				m_texture->restore_size();
			}
			else if (m_icon)
			{
				const std::string icon = { m_icon };
				if (get_icon_font())
					get_icon_font()->draw({ area.position.x + area.size.x * 0.5f, area.position.y + area.size.y * 0.5f }, m_style->get_color(is_enabled ? E_STYLE_COLOR::ACCENT : E_STYLE_COLOR::ELEMENT_GENERAL), icon, drawing::TEXTFLAG_CENTERED_X | drawing::TEXTFLAG_CENTERED_Y);
			}
			else
			{
				m_style->get_font(E_STYLE_FONT::MEDIUM)->draw({ area.position.x + area.size.x * 0.5f, area.position.y + area.size.y * 0.5f }, m_style->get_color(((get_is_background_dependant() || is_enabled) || is_hovered) ? E_STYLE_COLOR::ELEMENT_HIGHLIGHTED : E_STYLE_COLOR::ELEMENT_GENERAL), get_title_formatted(), drawing::TEXTFLAG_CENTERED_X | drawing::TEXTFLAG_CENTERED_Y);
				if (get_dynamic_size())
					set_size({ m_style->get_font(E_STYLE_FONT::MEDIUM)->get_text_size(get_title_formatted()).x + 5, std::max(m_style->get_font(E_STYLE_FONT::MEDIUM)->get_text_size(get_title_formatted()).y + 5, m_size.y) });
			}	
		}

		bool c_button::process_input(const input::input_capture_t& input, const float scroll)
		{
			if (input.key == VK_LBUTTON && input.get_key_state().is_pressed())
			{
				if (input::get_is_mouse_in_area(get_geometry_area()))
				{
					run_input_callback();
					think_input();
					return true;
				}
			}

			return false;
		}

		void c_button::update()
		{

		}
	}
}