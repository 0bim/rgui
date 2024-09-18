#pragma once
#ifndef C_TEXT_BOX_HPP
#define C_TEXT_BOX_HPP
#include "base/c_base_element.hpp"
#include "base/c_base_openable.hpp"
#include "base/c_base_callback.hpp"
#include "base/c_base_animated.hpp"
#include "base/c_base_variable_controller.hpp"

namespace rgui
{
	namespace elements
	{
		enum E_TB_FLAGS : std::uint8_t
		{
			TB_FLAG_NONE,
			TB_FLAG_NO_SPACE,
		    TB_FLAG_HIDDEN
		};

		class c_text_box : public c_base_element, public c_base_openable, public c_base_callback, public c_base_animated, public c_base_variable_controller<std::string>
		{
		public:
			c_text_box(const std::string& title, const math::point_t& position, const math::point_t& size = { 235, 25 });

		public:
			void draw() override;

			bool process_input(const input::input_capture_t& input, const float scroll) override;

			bool process_input(const char character) override;

			void update() override;

			inline bool is_text_highlighted() const;

			inline void erase_highlighted_text();

			inline void deselect_text();

			inline void clamp_selection();

			std::vector<std::string> get_text_split_lines() const;

		public:
			inline void set_port_area(const math::point_t& area) override final
			{
				c_base_element::set_port_area(area);
				set_size({ area.x, m_size.y });
			}

			inline void set_tb_flags(const std::uint8_t tb_flags)
			{
				m_tb_flags = tb_flags;
			}

		protected:
			std::size_t m_input_position;

			bool m_i_beam_flag;

			int m_highlight_begin, m_highlight_end;

			std::uint8_t m_tb_flags;

			//std::chrono::system_clock::time_point m_i_beam_timestamp;
		};
	}
}
#endif