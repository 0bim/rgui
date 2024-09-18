#include "c_text_box_multiline.hpp"

namespace rgui
{
	namespace elements
	{
		c_text_box_multiline::c_text_box_multiline(const std::string& title, const math::point_t& position, const math::point_t& size)
			: c_text_box(title, position, size), m_line_count(1), m_font(m_style->get_font(E_STYLE_FONT::SMALL))
		{

		}

		void c_text_box_multiline::draw()
		{
			const auto& area = get_geometry_area();
			drawing::draw_rectangle_filled(area, m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND), 7);

			drawing::draw_rectangle(area, m_style->get_color(E_STYLE_COLOR::GENERAL_OUTLINE), 7);
			const float outline_blend = ease_input(get_is_open());
			if (!m_input_animation.get_has_ended(m_input_smooth))
			{
				auto outline_color = m_style->get_color(E_STYLE_COLOR::ACCENT);
				outline_color.a *= outline_blend;
				drawing::draw_rectangle(area, outline_color, 7);
			}

			drawing::set_clip(area);

			auto small_font = m_style->get_font(E_STYLE_FONT::SMALL);
			small_font->draw({ area.position.x, area.position.y - 2 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL),get_title_formatted(), drawing::TEXTFLAG_BOTTOM);

			math::point_t line_size = { m_size.x, m_font->m_height * 2 };

			auto text_vec = get_text_split_lines();

			text_vec.resize(m_line_count);

			for (std::size_t i = 0; i < m_line_count; i++)
			{
				small_font->draw({ area.position.x + 5, area.position.y + line_size.y * i + line_size.y * 0.5f }, m_style->get_color(E_STYLE_COLOR::ACCENT), std::to_string(i + 1), drawing::TEXTFLAG_CENTERED_Y);
				m_font->draw({ area.position.x + 15, area.position.y + line_size.y * i + line_size.y * 0.5f }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL), text_vec[i], drawing::TEXTFLAG_CENTERED_Y);
			}

			drawing::pop_clip();
		}

		bool c_text_box_multiline::process_input(const input::input_capture_t& input, const float scroll)
		{
			if (input.get_key_state().is_pressed())
			{
				const auto& area = get_geometry_area();
				if (input.key == VK_LBUTTON)
				{
					if (input::get_is_mouse_in_area(area))
					{
						if (get_is_open())
						{
							auto text_vec = get_text_split_lines();
							text_vec.resize(m_line_count);

							math::point_t line_size = { m_size.x, m_font->m_height * 2 };

							std::size_t text_len = 0;

							const auto& cursor_position = input::get_mouse_pos();
							for (std::size_t i = 0; i < m_line_count; i++)
							{
								auto line_text_size = m_font->get_text_size(text_vec[i]);
								if (input::get_is_mouse_in_area({ { area.position.x + 15, area.position.y + line_size.y * i }, line_size }))
								{
									while (!text_vec[i].empty() && (m_position.x + 15 + line_text_size.x > cursor_position.x))
									{
										text_vec[i].pop_back();
										line_text_size = m_font->get_text_size(text_vec[i]);

										m_input_position = text_len + text_vec[i].size();
										break; //bad but whatever
									}
								}

								text_len += text_vec[i].size();
							}
						}
						else
							set_is_open(true);
					}
					else
						set_is_open(false);
				}
				else if (input.key == VK_RETURN)
				{
					m_variable->insert(m_variable->begin() + m_input_position, '\n');
					m_input_position++;
					m_line_count++;
				}

			}

			return false;
		}

		bool c_text_box_multiline::process_input(const char character)
		{
			if (get_is_open())
			{
				m_variable->insert(m_variable->begin() + m_input_position, character);
				if (character == '\n')
					m_line_count++;

				m_input_position++;

				return true;
			}

			return false;
		}

		void c_text_box_multiline::update()
		{

		}
	}
}