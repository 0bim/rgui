#pragma once
#ifndef C_STYLESHEET_HPP
#define C_STYLESHEET_HPP
#include <array>
#include "../../drawing/color/color.hpp"
#include "../../drawing/render/font_t.hpp"

namespace rgui
{
	namespace drawing
	{
		struct font_t;
	}

	namespace elements
	{
		enum class E_STYLE_COLOR
		{
			BACKGROUND = 0,
			BACKGROUND_LIGHT,
			BACKGROUND_DARK,
			ACCENT,
			GENERAL_OUTLINE,
			LIGHT_GREY,
			ELEMENT_GENERAL,
			ELEMENT_HIGHLIGHTED,
			ELEMENT_BACKGROUND,
			SCROLLBAR_BACKRGROUND,
			SCROLLBAR_FOREGROUND,
			MAX
		};

		enum class E_STYLE_FONT
		{
			VERY_SMALL,
			SMALL,
			MEDIUM,
			LARGE,
			VERY_LARGE,
			SMALL_ICON,
			MEDIUM_ICON,
			LARGE_ICON,
			WEAPON_ICON,
			MAX
		};

		class c_stylesheet
		{
		public:
			struct style_t
			{
				std::array<drawing::color_t, static_cast<std::size_t>(E_STYLE_COLOR::MAX)> m_colors = { };
				std::array<drawing::font_t*, static_cast<std::size_t>(E_STYLE_FONT::MAX)> m_fonts = { };

				drawing::color_t& get_color(const E_STYLE_COLOR index);

				const drawing::color_t& get_color(const E_STYLE_COLOR index) const;

				drawing::font_t* get_font(const E_STYLE_FONT index);

				void set_font(const E_STYLE_FONT index, drawing::font_t* font);
			};

			style_t& get_style();

		private:
			style_t m_style;
		};
	}
}
#endif