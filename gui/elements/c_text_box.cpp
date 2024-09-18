#include <sstream>
#include "c_text_box.hpp"

namespace rgui
{
	namespace elements
	{
		std::string get_clipboard()
		{
			if (!OpenClipboard(nullptr))
				return "";

			HANDLE hData = GetClipboardData(CF_TEXT);
			if (hData == nullptr)
				return "";

			char* c_text = static_cast<char*>(GlobalLock(hData));
			if (c_text == nullptr)
				return "";

			std::string ret = c_text;

			GlobalUnlock(hData);

			CloseClipboard();

			return ret;
		}

		inline void copy_to_clipboard(std::string_view string)
		{
			OpenClipboard(0);
			EmptyClipboard();
			HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, string.size() + 1);
			if (!hg) {
				CloseClipboard();
				return;
			}
			memcpy(GlobalLock(hg), string.data(), string.size());
			GlobalUnlock(hg);
			SetClipboardData(CF_TEXT, hg);
			CloseClipboard();
			GlobalFree(hg);
		}

		c_text_box::c_text_box(const std::string& title, const math::point_t& position, const math::point_t& size)
			: c_base_element(title, position, size), c_base_openable(), c_base_callback(), c_base_animated(math::easing::E_EASING::IN_OUT_SINE, 500, 500), c_base_variable_controller(), m_input_position(0), m_i_beam_flag(true), m_highlight_begin(0), m_highlight_end(0), m_tb_flags(TB_FLAG_NONE)
		{
			deselect_text();
		}

		void c_text_box::draw()
		{
			std::string backup_text = "";
			if (m_tb_flags & TB_FLAG_HIDDEN)
			{
				backup_text = get_value();
				set_value(std::string(backup_text.size(), '*'));
			}

			const auto& area = get_geometry_area();
			drawing::draw_rectangle_filled(area, m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND), m_rounding);

			//drawing::draw_rectangle(area, m_style->get_color(E_STYLE_COLOR::GENERAL_OUTLINE), m_rounding);
			const float outline_blend = ease_input(get_is_open());
			if (!m_input_animation.get_has_ended(m_input_smooth))
			{
				auto outline_color = m_style->get_color(E_STYLE_COLOR::ACCENT);
				outline_color.a *= outline_blend;
				drawing::draw_rectangle(area, outline_color, m_rounding);
			}

			auto small_font = m_style->get_font(E_STYLE_FONT::SMALL);
			small_font->draw({ area.position.x, area.position.y - 2 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL),get_title_formatted(), drawing::TEXTFLAG_BOTTOM);

			drawing::set_clip({ area.position, area.size - math::point_t(m_rounding, 0) });

			small_font->draw({ area.points.left + 5.f, area.points.top + area.size.y * 0.5f }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL), get_value(), drawing::TEXTFLAG_CENTERED_Y);

			/*std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
			if (duration_cast<milliseconds>(now - m_i_beam_timestamp).count() >= 500)
			{
				m_i_beam_timestamp = now;
				m_i_beam_flag ^= true;
			}*/

			auto get_position_by_index = [&](const std::size_t index)
			{
				const std::string_view cut = { m_variable->data(), m_variable->data() + index };
				std::string_view current_line = "";
				if (std::size_t index = cut.find_last_of('\n'); index != std::string_view::npos)
					current_line = cut.substr(index, cut.size() - index);

				const math::point_t& size = small_font->get_text_size(cut);
				const int x = area.position.x + (current_line.empty() ? size.x : small_font->get_text_size(current_line).x) + 5;
				const int y = area.position.y + std::max(0, size.y - area.size.y);

				return math::point_t(x, y);
			};

			if (m_is_open && m_i_beam_flag)
			{
				const auto& regular_i_beam_position = get_position_by_index(m_input_position);
				drawing::draw_line(regular_i_beam_position, { regular_i_beam_position.x, regular_i_beam_position.y + area.size.y - 2 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL));
			}

			if (is_text_highlighted())
			{
				drawing::color_t highlight_color = m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL);
				highlight_color.a = 100;

				const auto& lines = get_text_split_lines();
				int length_passed = 0;
				for (std::size_t i = 0; i < lines.size(); i++)
				{
					const auto& line = lines[i];
					auto begin_position = get_position_by_index(std::max(length_passed + 1, m_highlight_begin));
					if (length_passed > 0)
						begin_position.x = area.position.x + 5;

					length_passed += line.size();
					auto end_position = get_position_by_index(std::min(length_passed + 1, m_highlight_end));

					drawing::draw_rectangle_filled({ begin_position.x, begin_position.y + 2, end_position.x, end_position.y + area.size.y - 2 }, highlight_color);
				}
			}

			if (m_tb_flags & TB_FLAG_HIDDEN)
				set_value(backup_text);

			drawing::pop_clip();
		}

		bool c_text_box::process_input(const input::input_capture_t& input, const float scroll)
		{
			static std::chrono::system_clock::time_point delete_time_point = std::chrono::system_clock::now();

			if (m_tb_flags & TB_FLAG_NO_SPACE && input.key == VK_SPACE)
				return false;

			if (input.get_key_state().is_pressed())
			{
				delete_time_point = std::chrono::system_clock::now();

				//clamp_selection();

				if (m_is_open && input.key == 'A')
				{
					if (input::get_key_state(VK_CONTROL).is_held())
					{
						m_input_position = m_highlight_begin = 0;
						m_highlight_end = m_variable->size();
						return true;
					}
				}
				else if (m_is_open && input.key == 'C' && is_text_highlighted())
				{
					if (input::get_key_state(VK_CONTROL).is_held())
					{
						const int begin = std::min(m_highlight_begin, m_highlight_end), end = std::max(m_highlight_begin, m_highlight_end);
						std::string_view highlighted_text = { m_variable->begin() + begin, m_variable->begin() + end };
						copy_to_clipboard(highlighted_text);
						return true;
					}
				}
				else if (m_is_open && input.key == 'V')
				{
					if (input::get_key_state(VK_CONTROL).is_held())
					{
						if (m_style->get_font(E_STYLE_FONT::SMALL)->get_text_size(get_value()).x < m_size.x)
						{
							if (is_text_highlighted())
								erase_highlighted_text();

							const auto clipboard = get_clipboard();
							for (auto c : clipboard)
							{
								m_variable->insert(m_input_position++, std::string({ c }));
								if (m_style->get_font(E_STYLE_FONT::SMALL)->get_text_size(get_value()).x > m_size.x)
									break;
							}
						}

						return true;
					}
				}
				else if (input.key == VK_LBUTTON)
				{
					if (m_is_open)
					    deselect_text();
					if (m_is_open = input::get_is_mouse_in_area(get_geometry_area()))
						think_input();
					else
						run_input_callback(); //run callback on enter check for is_open is the callback function

					return m_is_open;
				}
				else if (input.key == VK_RETURN) //enter
				{
					if (m_is_open)
					{
						m_is_open = false;
						run_input_callback();
					}
					//m_variable->insert(m_input_position++, "\n");
					return true;
				}
				else if (m_is_open && get_is_target_element())
				{
					bool flag = false;
					if (input.key == VK_BACK || input.key == VK_DELETE)
					{
						if (!m_variable->empty())
						{
							if (is_text_highlighted())
								erase_highlighted_text();
							else if (m_input_position > 0)
								m_variable->erase(--m_input_position, 1);

							deselect_text();
						}

						run_input_callback();
						flag = true;
					}
					else if (flag = input.key == VK_LEFT)
						m_input_position--;
					else if (flag = input.key == VK_RIGHT)
						m_input_position++;

					m_input_position = std::clamp(m_input_position, 0u, m_variable->size());
					return flag;
				}
			}
			else if (m_is_open && input.get_key_state().is_held())
			{
				if (input.key == VK_BACK || input.key == VK_DELETE)
				{
					if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - delete_time_point).count() > 600)
					{
						if (!m_variable->empty())
						{
							if (is_text_highlighted())
								erase_highlighted_text();
							else if (m_input_position > 0)
							    m_variable->erase(--m_input_position, 1);

							run_input_callback();
						}
					}

					return true;
				}
				else if (input.key == VK_LBUTTON && (is_text_highlighted() || input::get_is_mouse_in_area(get_geometry_area())))
				{
					const math::point_t& cursor_position = input::get_mouse_pos();
					if (m_highlight_begin > -1)
					{
						std::size_t full_string_index = 0;

						for (auto& line : get_text_split_lines())
						{
							math::point_t text_size = m_style->get_font(E_STYLE_FONT::SMALL)->get_text_size(line);
							if (m_position.y + text_size.y > cursor_position.y)
							{
								while (!line.empty() && (m_position.x + text_size.x > cursor_position.x))
								{
									line.pop_back();
									text_size = m_style->get_font(E_STYLE_FONT::SMALL)->get_text_size(line);
								}
							}

							full_string_index += line.size();
						}

						m_highlight_end = full_string_index;
					}
					else
					{
						std::size_t full_string_index = 0;
						for (auto& line : get_text_split_lines())
						{
							math::point_t text_size = m_style->get_font(E_STYLE_FONT::SMALL)->get_text_size(line);
							if (m_position.y + text_size.y > cursor_position.y)
							{
								while (!line.empty() && (m_position.x + text_size.x > cursor_position.x))
								{
									line.pop_back();
									text_size = m_style->get_font(E_STYLE_FONT::SMALL)->get_text_size(line);
								}
							}

							full_string_index += line.size();
						}

						m_highlight_begin = full_string_index;
					}

					m_input_position = std::max(0, std::min(m_highlight_begin, m_highlight_end));
					return true;
				}
			}

			return false;
		}

		bool c_text_box::process_input(const char character)
		{
			if (m_tb_flags & TB_FLAG_NO_SPACE && character == ' ')
				return false;

			if (m_is_open && get_is_target_element())
			{
				if (character >= 32 && character <= 127)
				{
					if (is_text_highlighted())
					{
						erase_highlighted_text();
						deselect_text();
					}

					if (m_style->get_font(E_STYLE_FONT::SMALL)->get_text_size(get_value()).x < m_size.x)
					    m_variable->insert(std::min(m_input_position++, m_variable->size()), { character });

					run_input_callback();
					return true;
				}
			}
			return false;
		}

		void c_text_box::update()
		{

		}

		bool c_text_box::is_text_highlighted() const
		{
			return m_highlight_begin > -1 && m_highlight_end > -1 && m_highlight_begin != m_highlight_end;
		}

		void c_text_box::erase_highlighted_text()
		{
			const int begin = std::min(m_highlight_begin, m_highlight_end), end = std::max(m_highlight_begin, m_highlight_end) - begin;
			m_variable->erase(begin, end);
			m_input_position = std::clamp(m_input_position, 0u, m_variable->size());
		}

		void c_text_box::deselect_text()
		{
			m_highlight_begin = m_highlight_end = -1;
		}

		void c_text_box::clamp_selection()
		{
			const int max = m_variable->size();
			m_highlight_begin = std::clamp(m_highlight_begin, -1, max);
			//m_input_position = m_highlight_end = std::clamp(m_highlight_end, -1, max);
		}

		std::vector<std::string> c_text_box::get_text_split_lines() const
		{
			std::vector<std::string> lines = { };
			std::istringstream iss(get_value());
			std::string temp_line = "";
			while (std::getline(iss, temp_line))
				lines.push_back(temp_line);

			//make sure this has atleast 1 line to avoid crashing
			if (lines.empty())
				lines.resize(1);

			return lines;
		}
	}
}

