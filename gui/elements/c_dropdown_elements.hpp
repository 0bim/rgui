#pragma once
#ifndef C_DROPDOWN_ELEMENTS_HPP
#define C_DROPDOWN_ELEMENTS_HPP
#include "base/c_base_parent.hpp"
#include "base/c_base_openable.hpp"
#include "base/c_base_animated.hpp"

namespace rgui
{
	namespace elements
	{
		class c_dropdown_elements : public c_base_parent, public c_base_openable, public c_base_animated
		{
		public:
			c_dropdown_elements(const std::string& title, const int child_panel_height, const math::point_t& position, const math::point_t& size = { 235, 23 });

		public:
			void draw() override final;

			bool process_input(const input::input_capture_t& input, const float scroll) override final;

			void update() override final;

			math::point_t calculate_auto_position(const int space_between, const bool sameline = false) const override final;

			std::shared_ptr<c_checkbox> checkbox(const std::string& title, bool* variable = nullptr, const int space_between = 5) override final;

			std::shared_ptr<c_colorpicker> colorpicker(const std::string& title, drawing::color_t* variable = nullptr, const bool draw_title = false, const bool same_line = false, const int space_between = -2, const int margin = 20) override final;

		public:
			inline math::point_t get_children_size() const override final
			{
				math::point_t ret = c_base_parent::get_children_size();
				return { ret.x, ret.y + m_scroll - 6 };
			}

		private:
			int m_child_panel_height;
		};
	}
}

#endif