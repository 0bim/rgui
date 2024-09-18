#pragma once
#ifndef C_CHECKBOX_HPP
#define C_CHECKBOX_HPP
#include "base/c_base_element.hpp"
#include "base/c_base_animated.hpp"
#include "base/c_base_variable_controller.hpp"
#include "base/c_base_value_change_callback.hpp"

namespace rgui
{
	namespace elements
	{
		class c_checkbox : public c_base_element, public c_base_animated, public c_base_variable_controller<bool>, public c_base_value_change_callback<bool>
		{
		public:
			c_checkbox(const std::string& title, const math::point_t& position, const math::point_t& size = { 10, 10 });

		public:
			void draw() override final;

			bool process_input(const input::input_capture_t& input, const float scroll) override final;

			void update() override final;

		private:
		};
	}
}

#endif