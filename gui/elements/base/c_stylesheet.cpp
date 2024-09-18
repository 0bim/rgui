#include "c_stylesheet.hpp"
#include "../../drawing/render/render.hpp"

namespace rgui
{
	namespace elements
	{
		drawing::color_t& c_stylesheet::style_t::get_color(const E_STYLE_COLOR index)
		{
			return m_colors.at(static_cast<std::size_t>(index));
		}

		const drawing::color_t& c_stylesheet::style_t::get_color(const E_STYLE_COLOR index) const
		{
			return m_colors.at(static_cast<std::size_t>(index));
		}

		drawing::font_t* c_stylesheet::style_t::get_font(const E_STYLE_FONT index)
		{
			return m_fonts.at(static_cast<std::size_t>(index));
		}

		void c_stylesheet::style_t::set_font(const E_STYLE_FONT index, drawing::font_t* font)
		{
			m_fonts.at(static_cast<std::size_t>(index)) = font;
		}

		c_stylesheet::style_t& c_stylesheet::get_style()
		{
			return m_style;
		}
	}
}