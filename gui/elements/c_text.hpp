#pragma once
#ifndef C_TEXT_HPP
#define C_TEXT_HPP
#include "base/c_base_element.hpp"

namespace rgui
{
	namespace elements
	{
		class c_text : public c_base_element
		{
		public:
			c_text(const std::string& title, const math::point_t& position, drawing::font_t* font, const E_STYLE_COLOR color_index, const std::uint8_t text_flags);

		public:
			void draw() override final;

			bool process_input(const input::input_capture_t& input, const float scroll) override final;

			void update() override final;

		public:
			inline void set_title(const std::string_view title) override final
			{
				c_base_element::set_title(title);
				set_size(get_font()->get_text_size(get_title_formatted()));
			}

			inline void set_font(drawing::font_t* font)
			{
				m_font = font;
			}

			inline drawing::font_t* get_font()
			{
				return m_font;
			}

		private:
			drawing::font_t* m_font;
			std::uint8_t m_text_flags;
			E_STYLE_COLOR m_color_index;
		};
	}
}

#endif