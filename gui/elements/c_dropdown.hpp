#pragma once
#ifndef C_DROPDOWN_HPP
#define C_DROPDOWN_HPP
#include "base/c_base_element.hpp"
#include "base/c_base_selectable.hpp"
#include "base/c_base_openable.hpp"
#include "base/c_base_icon.hpp"
#include "base/c_base_animated.hpp"
#include "base/c_base_scrollable.hpp"

namespace rgui
{
	namespace elements
	{
		class c_dropdown : public c_base_element, public c_base_selectable, public c_base_openable, public c_base_animated, public c_base_scrollable
		{
		public:
			c_dropdown(const std::string& title, const math::point_t& position, const math::point_t& size = { 235, 25 });

		public:
			void draw() override final;

			bool process_input(const input::input_capture_t& input, const float scroll) override final;

			void update() override final;

		public:
			shapes::rect_t get_options_area(const bool ignore_clip);

		public:
			inline void set_options_clip_limit(const std::size_t limit)
			{
				m_option_clip_limit = limit;
			}

			inline void set_is_enabled_callback(const std::function<bool(const std::size_t index)> callback)
			{
				c_base_index_callback::set_is_enabled_callback(callback);
			}

			inline void on_retarget() override final
			{
				if (m_is_open)
					think_input();
				m_is_open = false;
			}

			inline void toggle_is_open() override final
			{
				c_base_openable::toggle_is_open();
				think_input();
			}

			inline void set_port_area(const math::point_t& area) override final
			{
				c_base_element::set_port_area(area);
				set_size({ area.x, m_size.y });
			}

		private:
			std::size_t m_option_clip_limit;
		};
	}
}

#endif