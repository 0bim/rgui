#pragma once
#ifndef C_TEXT_BOX_MULTILINE
#define C_TEXT_BOX_MULTILINE
#include "c_text_box.hpp"

namespace rgui
{
	namespace elements
	{
		class c_text_box_multiline : public c_text_box
		{
		public:
			c_text_box_multiline(const std::string& title, const math::point_t& position, const math::point_t& size);

		public:
			void draw() override final;

			bool process_input(const input::input_capture_t& input, const float scroll) override final;

			bool process_input(const char character) override final;

			void update() override final;

		public:
			std::size_t m_line_count;

			drawing::font_t* m_font;
		};
	}
}

#endif