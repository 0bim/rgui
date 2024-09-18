#include "c_input_box.hpp"

namespace rgui
{
	namespace elements
	{
		static std::string m_input_str = "";

		template<input_number_t T>
		void c_input_box<T>::draw()
		{
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
			small_font->draw({ m_position.x, m_position.y - 2 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL),get_title_formatted(), drawing::TEXTFLAG_BOTTOM);

			std::string value_text = "";
			if (m_is_open)
				value_text = m_input_str;
			else
			{
				char buffer[256];
				std::sprintf(buffer, std::is_same_v<T, float> ? "%.7f" : "%d", this->get_value());
				value_text = std::string(buffer);
			}

			drawing::set_clip(area);

			small_font->draw({ area.points.left + 5.f, area.points.top + area.size.y * 0.5f }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL), value_text, drawing::TEXTFLAG_CENTERED_Y);

			drawing::pop_clip();
		}

		template<input_number_t T>
		bool c_input_box<T>::process_input(const input::input_capture_t& input, const float scroll)
		{
			auto accept_input = [&]()
			{
				if (!m_input_str.empty())
				{
					std::stringstream stream(m_input_str);
					T x;
					stream >> x;
					this->set_value(x);
					run_input_callback();
				}

				m_input_str.clear();
				m_is_open = false;
			};

			if (input.get_key_state().is_pressed())
			{
				if (input.key == VK_LBUTTON)
				{
					if (input::get_is_mouse_in_area(get_geometry_area()))
					{
						m_input_str.clear();
						m_is_open = true;
						think_input();
						return true;
					}
					else if (m_is_open && get_is_target_element())
						accept_input();
				}
				else if (m_is_open && get_is_target_element())
				{
					if (input.key == VK_RETURN)
					{
						accept_input();
						return true;
					}
					else if (input.key == VK_ESCAPE)
					{
						m_input_str.clear();
						m_is_open = false;
						return true;
					}
					else if (input.key == VK_BACK || input.key == VK_DELETE)
					{
						if (!m_input_str.empty())
						    m_input_str.pop_back();

						return true;
					}
				}
			}


			return false;
		}

		template<input_number_t T>
		bool c_input_box<T>::process_input(const char character)
		{
			if (m_is_open && get_is_target_element())
			{
				if (std::isdigit(character) || (character == '.' && m_input_str.find('.') == std::string::npos))
					m_input_str.push_back(character);
				return true;
			}

			return false;
		}

		template<input_number_t T>
		void c_input_box<T>::update()
		{

		}

		template void c_input_box<float>::draw();
		template bool c_input_box<float>::process_input(const input::input_capture_t& input, const float scroll);
		template bool c_input_box<float>::process_input(const char character);
		template void c_input_box<float>::update();

		template void c_input_box<int>::draw();
		template bool c_input_box<int>::process_input(const input::input_capture_t& input, const float scroll);
		template bool c_input_box<int>::process_input(const char character);
		template void c_input_box<int>::update();

		template void c_input_box<std::uint8_t>::draw();
		template bool c_input_box<std::uint8_t>::process_input(const input::input_capture_t& input, const float scroll);
		template bool c_input_box<std::uint8_t>::process_input(const char character);
		template void c_input_box<std::uint8_t>::update();
	}
}