#include "c_dropdown.hpp"

namespace rgui
{
	namespace elements
	{
		c_dropdown::c_dropdown(const std::string& title, const math::point_t& position, const math::point_t& size)
			: c_base_element(title, position, size), c_base_selectable(), c_base_openable(), c_base_animated(math::easing::E_EASING::OUT_SINE, 400, 200), c_base_scrollable(), m_option_clip_limit(6)
		{

		}

		void c_dropdown::draw()
		{
			const auto& geometry_area = get_geometry_area();

			const float blend_input = ease_input(!get_is_open());

			const auto& background_color = m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND).interpolate(m_style->get_color(E_STYLE_COLOR::ACCENT), blend_input);
			drawing::draw_rectangle_filled(geometry_area, background_color, m_rounding);
			//drawing::draw_rectangle(geometry_area, m_style->get_color(E_STYLE_COLOR::GENERAL_OUTLINE), m_rounding);

			//m_style->get_font(E_STYLE_FONT::SMALL_ICON)->draw({ geometry_area.points.right - 5, geometry_area.points.top + geometry_area.size.y / 2 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL), { m_icon }, drawing::TEXTFLAG_LEFT | drawing::TEXTFLAG_CENTERED_Y);

			/*int line_length = 8, line_height = geometry_area.size.y / 2 - 3;
			for (std::size_t i = 0; i < 4; i++)
			{
				drawing::draw_line({ geometry_area.points.right - line_length - 5, geometry_area.position.y + line_height }, { geometry_area.points.right - 5, geometry_area.position.y + line_height }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL));
				line_height += 2;
			}*/

			const bool is_hovered = input::get_is_mouse_in_area(geometry_area);

			auto small_font = m_style->get_font(E_STYLE_FONT::SMALL);

			small_font->draw({ geometry_area.position.x, geometry_area.position.y - 2 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL).interpolate(m_style->get_color(E_STYLE_COLOR::ELEMENT_HIGHLIGHTED), blend_input), get_title_formatted(), drawing::TEXTFLAG_BOTTOM);

			//const bool clip = m_options.size() > m_option_clip_limit;

			auto displayable_options_area = get_options_area(false), complete_options_area = get_options_area(true);
			const bool has_animation_ended = m_input_animation.get_has_ended(200);
			bool run_hover_animation = input::get_is_mouse_in_area(geometry_area);
			if (get_is_open() || !has_animation_ended)
			{
				//draw_input_fill_animation(geometry_area, m_style->get_color(E_STYLE_COLOR::GENERAL_OUTLINE), ease_input());

				run_hover_animation = false; //now reverse the animation because it's already opened

				displayable_options_area.size.y *= blend_input;
				displayable_options_area.update_points();

				drawing::set_clip({ displayable_options_area.position - m_rounding, displayable_options_area.size + m_rounding }, true);

				drawing::draw_rectangle_filled(displayable_options_area, m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND), m_rounding);
				//drawing::draw_rectangle(displayable_options_area, m_style->get_color(E_STYLE_COLOR::GENERAL_OUTLINE), m_rounding);

				drawing::pop_clip();

				drawing::set_clip({ displayable_options_area.position, displayable_options_area.size - math::point_t(-6, 0) }, true);

				const static int spacing_between = 2;
				int padding = 5;
				for (std::size_t i = 0; i < m_options.size(); i++)
				{
					const std::string_view option = m_options.at(i);

					if (option.find(m_filter) == std::string_view::npos)
						continue;

					const math::point_t& text_size = small_font->get_text_size(option);
					const math::point_t text_pos = { geometry_area.position.x + 5, geometry_area.position.y + geometry_area.size.y + padding - static_cast<int>(m_scroll) };

					/*if (text_pos.y + text_size.y > displayable_options_area.points.bottom)
						break; //sorry
					else if (text_pos.y > displayable_options_area.points.top)*/
					small_font->draw(text_pos, m_style->get_color(get_is_enabled(i) ? E_STYLE_COLOR::ACCENT : (get_is_selected(i) ? E_STYLE_COLOR::ELEMENT_HIGHLIGHTED : E_STYLE_COLOR::ELEMENT_GENERAL)), option);
					if (get_is_enabled(i))
					{
						const math::point_t checkmark_size = { 10, 10 };
						drawing::draw_checkmark({ displayable_options_area.points.right - checkmark_size.x - 12, text_pos.y }, checkmark_size, m_style->get_color(E_STYLE_COLOR::ACCENT));
					}

					padding += text_size.y + spacing_between;
				}

				drawing::pop_clip();
			}

			std::string selected_options = "";
			bool should_shade = false;

			for (std::size_t i = 0; i < m_options.size(); i++)
			{
				const std::string_view option = m_options.at(i);
				if (get_is_enabled(i))
					selected_options += std::string(selected_options.empty() ? "" : ", ").append(option);
			}

			while (small_font->get_text_size(selected_options).x > get_size().x - 25 && !selected_options.empty())
			{
				selected_options.erase(selected_options.end() - 1);
				should_shade = true;
			}

			const float blend_hover = think_hover(run_hover_animation);

			small_font->draw({ geometry_area.position.x + 5, geometry_area.position.y + geometry_area.size.y * 0.5f + 2 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL).interpolate(m_style->get_color((is_hovered && !get_is_open()) ? E_STYLE_COLOR::ELEMENT_HIGHLIGHTED : E_STYLE_COLOR::ELEMENT_BACKGROUND), (is_hovered && !get_is_open()) ? blend_hover : blend_input), selected_options, drawing::TEXTFLAG_CENTERED_Y);
			if (should_shade)
			{
				drawing::color_t shade_color = background_color, shade_color_gradient = shade_color;
				shade_color.a = 0;
				shade_color_gradient.a = m_style->get_color(E_STYLE_COLOR::ELEMENT_HIGHLIGHTED).a;
				drawing::draw_rectangle_filled({ { geometry_area.position.x + geometry_area.size.x * 0.5f, geometry_area.position.y + 1 }, { geometry_area.size.x * 0.5f, geometry_area.size.y - 2 } }, shade_color, shade_color_gradient, true, m_rounding);
			}

			//hover outline
			{
				drawing::color_t outline_color = m_style->get_color(E_STYLE_COLOR::ACCENT);
				outline_color.a *= blend_hover;
				drawing::draw_rectangle(geometry_area, outline_color, m_rounding);
			}

			//draw scrollbar
			if (m_is_open)
			{
				drawing::draw_scrollbar(displayable_options_area.size.y, complete_options_area.size.y, m_scroll, { { displayable_options_area.points.right - 8, displayable_options_area.points.top + m_rounding }, { 2, (displayable_options_area.size.y - m_rounding * 2) * blend_input } }, m_rounding, false, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL), m_style->get_color(E_STYLE_COLOR::ACCENT));
			}

			drawing::set_rotation(180.f * blend_input);

			m_style->get_font(E_STYLE_FONT::SMALL_ICON)->draw({ geometry_area.points.right - 17, geometry_area.position.y + geometry_area.size.y * 0.5f }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL).interpolate(m_style->get_color((is_hovered && !get_is_open()) ? E_STYLE_COLOR::ACCENT : E_STYLE_COLOR::ELEMENT_BACKGROUND), (is_hovered && !get_is_open()) ? blend_hover : blend_input), std::string("h"), drawing::TEXTFLAG_CENTERED_X | drawing::TEXTFLAG_CENTERED_Y);

			drawing::pop_rotation();
		}

		bool c_dropdown::process_input(const input::input_capture_t& input, const float scroll)
		{
			auto small_font = m_style->get_font(E_STYLE_FONT::SMALL);

			if (scroll != 0.f && m_is_open)
			{
				/*auto complete_options_area = get_options_area(true), displayable_options_area = get_options_area(false);
				m_scroll += scroll;
				m_scroll = std::clamp(m_scroll, 0.f, static_cast<float>(complete_options_area.size.y - displayable_options_area.size.y));*/

				process_scroll(scroll, get_options_area(false).size, get_options_area(true).size);
				return true;
			}

			if (input.key == VK_LBUTTON && input.get_key_state().is_pressed())
			{
				if (m_is_open)
				{
					const auto& options_area = get_options_area(false);

					if (input::get_is_mouse_in_area(options_area))
					{
						const static int spacing_between = 2;
						int padding = 5;
						for (std::size_t i = 0; i < m_options.size(); i++)
						{
							const std::string_view option = m_options.at(i);
							if (option.find(m_filter) == std::string_view::npos)
								continue;

							const math::point_t text_size = small_font->get_text_size(option);
							//TODO: block out of bounds options
							const auto& geometry_area = get_geometry_area();
							const shapes::rect_t option_area = { { geometry_area.position.x, geometry_area.position.y + geometry_area.size.y + padding - static_cast<int>(m_scroll) }, { geometry_area.size.x, text_size.y } };

							if (option_area.points.top + m_scroll > options_area.points.top && option_area.points.bottom - m_scroll < options_area.points.bottom)
							{
								if (input::get_is_mouse_in_area(option_area))
								{
									run_input_callback(i);
									return true;
								}

								padding += text_size.y + spacing_between;
							}
						}

						return true;
					}
				}
				if (input::get_is_mouse_in_area(get_geometry_area()))
				{
					toggle_is_open();
					return true;
				}
				else
				{
					if (m_is_open)
					{
						m_is_open = false;
						think_input();
						return false; //return false so that other things are clickable
					}
				}
			}

			return m_is_open;
		}

		void c_dropdown::update()
		{

		}

		shapes::rect_t c_dropdown::get_options_area(const bool ignore_clip)
		{
			auto small_font = m_style->get_font(E_STYLE_FONT::SMALL);

			math::point_t option_area_size = { get_size().x, 0 };

			const static int spacing_between = 2;
			int padding = 5;
			for (std::size_t i = 0; i < m_options.size(); i++)
			{
				const math::point_t text_size = small_font->get_text_size(m_options.at(i));

				if (i < m_option_clip_limit || ignore_clip) //if too many option then stop adding to the box size because it's gonna be clamped anyway.
					option_area_size.y += text_size.y + spacing_between;
			}

			option_area_size.y += 10; //5px on top and 5px on bottom

			return { { get_position().x, get_position().y + get_size().y + 2 }, option_area_size };
		}
	}
}
