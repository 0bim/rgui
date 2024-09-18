#include "c_text.hpp"

namespace rgui
{
	namespace elements
	{
		c_text::c_text(const std::string& title, const math::point_t& position, drawing::font_t* font, const E_STYLE_COLOR color_index, const std::uint8_t text_flags)
			: c_base_element(title, position, { 0, 0 }), m_font(font), m_color_index(color_index), m_text_flags(text_flags)
		{
			set_title(title); //updates the size with the title size according to the font size
		}

		void c_text::draw()
		{
			m_font->draw(get_position(), m_style->get_color(m_color_index),get_title_formatted(), m_text_flags);
		}

		bool c_text::process_input(const input::input_capture_t& input, const float scroll)
		{
			return false;
		}

		void c_text::update()
		{

		}
	}
}