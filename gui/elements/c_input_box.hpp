#pragma once
#ifndef C_INPUT_BOX_HPP
#define C_INPUT_BOX_HPP
#include "base/c_base_element.hpp"
#include "base/c_base_openable.hpp"
#include "base/c_base_callback.hpp"
#include "base/c_base_animated.hpp"
#include "base/c_base_variable_controller.hpp"

namespace rgui
{
	namespace elements
	{
		template<typename T>
		concept input_number_t = std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, std::uint8_t>;

		template<input_number_t T>
		class c_input_box : public c_base_element, public c_base_openable, public c_base_callback, public c_base_animated, public c_base_variable_controller<T>
		{
		public:
			c_input_box(const std::string& title, const math::point_t& position, const math::point_t& size = { 235, 20 })
				: c_base_element(title, position, size), c_base_openable(), c_base_callback(), c_base_animated(math::easing::E_EASING::IN_OUT_SINE, 500, 500), c_base_variable_controller<T>()
			{

			}

		public:
			void draw() override final;

			bool process_input(const input::input_capture_t& input, const float scroll) override final;

			bool process_input(const char character) override final;

			void update() override final;

		public:
			inline void set_port_area(const math::point_t& area) override final
			{
				c_base_element::set_port_area(area);
				set_size({ area.x, m_size.y });
			}
		};
	}
}

#endif