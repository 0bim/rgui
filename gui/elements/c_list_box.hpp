#pragma once
#ifndef C_LIST_BOX_HPP
#define C_LIST_BOX_HPP
#include "base/c_base_element.hpp"
#include "base/c_base_selectable.hpp"
#include "base/c_base_openable.hpp"
#include "base/c_base_animated.hpp"
#include "base/c_base_scrollable.hpp"

namespace rgui
{
	namespace elements
	{
		/*
		* NOTE:
		* make a base class called c_base_option_list
		* add a virtual method called draw_option
		* move the get_option_size from c_dropdown to that c_base_option_list and make it a virtual function
		*/
		class c_list_box : public c_base_element, public c_base_selectable, public c_base_animated, public c_base_scrollable
		{
		public:
			c_list_box(const std::string& title, const math::point_t& position, const math::point_t& size = { 235, 14 });

		public:
			void draw() override final;

			bool process_input(const input::input_capture_t& input, const float scroll) override final;

			void update() override final;

		public:
			inline void on_options_resize() override
			{
				//m_size = get_options_area(true, true).size;
				//m_size.y = std::clamp(m_size.y, 0, 120);
			}

			inline virtual void draw_option(const std::size_t index, const math::point_t& position)
			{
				m_style->get_font(E_STYLE_FONT::SMALL)->draw(position, m_style->get_color(get_is_selected(index) ? E_STYLE_COLOR::ELEMENT_HIGHLIGHTED : (get_is_enabled(index) ? E_STYLE_COLOR::ACCENT : E_STYLE_COLOR::ELEMENT_GENERAL)), m_options.at(index));
			}

			inline virtual math::point_t get_option_size(const std::size_t index)
			{
				return m_style->get_font(E_STYLE_FONT::SMALL)->get_text_size(m_options.at(index));
			}

			inline void set_port_area(const math::point_t& area) override final
			{
				c_base_element::set_port_area(area);
				set_size({ area.x, m_size.y });
			}

		protected:

			shapes::rect_t get_options_area(const bool ignore_clip, const bool ignore_scroll);
		};
	}
}

#endif