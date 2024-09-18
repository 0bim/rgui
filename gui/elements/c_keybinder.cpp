#include <sstream>
#include "c_keybinder.hpp"
#include "base/c_base_parent.hpp"
#include "../helpers/settings/settings.hpp"

namespace rgui
{
	namespace elements
	{
		c_keybinder::c_keybinder(const std::string& title, const math::point_t& position)
			: c_base_element(title, position, math::point_t(20, 10)), c_base_openable(), m_mode(0)
		{
			m_mode_dropdown = std::make_shared<c_dropdown>("", position, math::point_t(20, 14));
			m_mode_dropdown->set_options({ "always", "hold", "toggle" });
			m_mode_dropdown->set_size({ m_style->get_font(E_STYLE_FONT::VERY_SMALL)->get_text_size(std::string("always")).x * 2, m_mode_dropdown->get_size().y });
			m_mode_dropdown->set_input_callback([&](const std::size_t mode)
				{
					m_mode = mode;
				});
			m_mode_dropdown->set_is_enabled_callback([&](const std::size_t index)
				{
					return m_mode == index;
				});
		}

		void c_keybinder::draw()
		{
			auto small_font = m_style->get_font(E_STYLE_FONT::VERY_SMALL);

			//small_font->draw({ m_parent->get_port_area().x, get_position().y + get_size().y * 0.5f}, m_style->get_color(m_is_open ? E_STYLE_COLOR::ACCENT : E_STYLE_COLOR::ELEMENT_GENERAL),get_title_formatted(), drawing::TEXTFLAG_CENTERED_Y);
			//if is open or stil in the process of closing then draw the dropdown
			if (m_mode_dropdown->get_is_open() || !m_mode_dropdown->get_input_animation().get_has_ended(150))
				m_mode_dropdown->draw();
			else
			{
				std::size_t key = m_variable->input.key;
				//PLEASE INCLUDE FMT OR USE SPRINTF
				std::stringstream stream;
				stream << "[" << (!m_is_open && key > 0 && key < settings::m_key_names.size() ? (" " + settings::m_key_names[key] + " ") : "   ") << "]";
				m_size = small_font->get_text_size(stream.str()); //dynamically set new size to match the key's name inside the brackets
				small_font->draw({ get_position().x + 3, get_position().y }, m_style->get_color(m_is_open ? E_STYLE_COLOR::ACCENT : E_STYLE_COLOR::ELEMENT_GENERAL), stream.str(), drawing::TEXTFLAG_LEFT);
			}
		}

		bool c_keybinder::process_input(const input::input_capture_t& input, const float scroll)
		{
			if (input.key < 0 || input.key > 256 || !input.get_key_state().is_pressed())
				return false;

			//process input on the mode selection dropdown
			if (m_mode_dropdown->get_is_open())
			{
				if (m_mode_dropdown->process_input(input, scroll))
				{
					m_variable->input.state = static_cast<input::E_KEYSTATES>(m_mode);
					m_variable->flag = false; //if anything changed it should check again
					return true;
				}

				return false;
			}
			else
			{
				const bool is_visible = get_is_visible();
				//if waiting for key to bind
				if (m_is_open && is_visible)
				{
					//we don't wanna catch escape
					m_variable->input.key = input.key == VK_ESCAPE ? 0 : input.key;
					m_variable->flag = false; //if anything changed it should check again
					m_is_open = false;
					return true;
				}
				else
				{
					const auto& area = get_geometry_area();
					//is mouse in area + account for inaccurate get_text_size
					if (is_visible && input::get_is_mouse_in_area({ { area.position.x - area.size.x, area.position.y }, area.size }))
					{
						if (input.key == VK_LBUTTON)
						{
							m_is_open = true;
							return true;
						}
						else if (input.key == VK_RBUTTON)
						{
							m_mode_dropdown->toggle_is_open(); //this will open/close (in this case open) it and begin the animation
							m_mode_dropdown->set_position({ area.position.x - m_mode_dropdown->get_size().x, area.position.y }); //this will reposition the dropdown
							return true;
						}
					}
				}
			}

			return false;
		}

		void c_keybinder::update()
		{
			if (m_variable)
			{
				if (!m_variable->input.key || m_variable->input.state == input::E_KEYSTATES::NONE) //awls
					m_variable->flag = true;
				else
				{
					if (m_variable->input.state == input::E_KEYSTATES::PRESSED) //toggle
					{
						if (input::get_key_state(m_variable->input.key).is_pressed())
							m_variable->flag ^= true;
					}
					else //hold
						m_variable->flag = !input::get_key_state(m_variable->input.key).is_idle();
				}
			}
		}
	}
}