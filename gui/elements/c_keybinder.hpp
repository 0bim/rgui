#pragma once
#ifndef C_KEYBINDER_HPP
#define C_KEYBINDER_HPP
#include "base/c_base_element.hpp"
#include "base/c_base_variable_controller.hpp"
#include "base/c_base_openable.hpp"
#include "c_dropdown.hpp"

namespace rgui
{
	namespace elements
	{
		class c_keybinder : public c_base_element, public c_base_openable, public c_base_variable_controller<input::custom_keybind_t>
		{
		public:
			c_keybinder(const std::string& title, const math::point_t& position);

		public:
			void draw() override final;

			bool process_input(const input::input_capture_t& input, const float scroll) override final;

			void update() override final;
			
		public:
			inline void set_position(const math::point_t& position) override final
			{
				c_base_element::set_position(position);
				m_mode_dropdown->set_position(position);
			}

			inline void on_move(const math::point_t& delta) override
			{
				m_position += delta;
				m_mode_dropdown->on_move(delta);
			}

			inline void on_retarget() override final
			{
				m_is_open = false;
			}

			inline void set_variable(input::custom_keybind_t* variable) override final
			{
				c_base_variable_controller::set_variable(variable);
				m_mode = !!variable ? static_cast<int>(variable->input.state) : 0;
			}

		private:
			std::shared_ptr<c_dropdown> m_mode_dropdown;
			int m_mode;
		};
	}
}

#endif