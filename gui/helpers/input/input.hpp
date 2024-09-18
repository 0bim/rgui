#pragma once

#ifndef RGUI_INPUT_HPP
#define RGUI_INPUT_HPP

#include <array>
#include <functional>
#include <string_view>
#include "../math/math.hpp"
#include "../math/point_t.hpp"
#include "../shapes/shapes.hpp"

#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_BACK 0x08
#define VK_SHIFT 0x10
#define VK_ESCAPE 0x1B
#define VK_SPACE 0x20
#define VK_LEFT 0x25
#define VK_RIGHT 0x27
#define VK_INSERT 0x2D
#define VK_DELETE 0x2E

namespace rgui
{
	namespace input
	{
		enum class E_KEYSTATES
		{
			NONE = 0,
			HELD,
			PRESSED
		};

		struct key_states_t
		{
			E_KEYSTATES m_key_state;

			key_states_t(const E_KEYSTATES key_state = E_KEYSTATES::NONE)
				: m_key_state(key_state)
			{

			}

			//--------------------------------------------------//

			inline bool is_pressed() const
			{
				return m_key_state == E_KEYSTATES::PRESSED;
			}

			inline bool is_held() const
			{
				return m_key_state == E_KEYSTATES::HELD;
			}

			inline bool is_idle() const
			{
				return m_key_state == E_KEYSTATES::NONE;
			}
		};

		struct keybind_t
		{
			keybind_t() 
				: state(E_KEYSTATES::NONE), callback(nullptr)
			{ 

			}

			keybind_t(E_KEYSTATES state, std::function<void()> callback)
				: state(state), callback(callback) { }

			E_KEYSTATES state;
			std::function<void()> callback;
		};


		struct input_capture_t
		{
			input_capture_t()
				: key(0), state(E_KEYSTATES::HELD)
			{
			}

			input_capture_t(std::size_t key, E_KEYSTATES state) 
				: key(key), state(state)
			{
			}

			inline bool operator==(input_capture_t other) const
			{
				return key == other.key && state == other.state;
			}

			inline key_states_t get_key_state() const
			{
				return key_states_t(state);
			}

			std::size_t key;
			E_KEYSTATES state; //held || pressed etc
		};

		struct custom_keybind_t
		{
			custom_keybind_t()
			{
				input.key = 0;
				input.state = E_KEYSTATES::HELD;
				flag = false;
			}

			custom_keybind_t(input_capture_t input, bool flag) : input(input), flag(flag)
			{

			}

			custom_keybind_t(const std::size_t key, const int state)
				: input({ key, E_KEYSTATES(state) }), flag(false)
			{

			}

			inline bool operator==(custom_keybind_t other) const
			{
				return input == other.input;
			}

			inline bool operator!=(custom_keybind_t other) const
			{
				return !(input == other.input);
			}

			input_capture_t input;
			bool flag;
		};

		bool get_is_mouse_in_area(const shapes::rect_t& rect);
		//bool get_is_component_hovered(std::shared_ptr<components::c_component> control);

		void set_mouse_pos(const math::point_t& pos);
		math::point_t get_mouse_pos();

		void set_key_state(const int key, const E_KEYSTATES state);

		key_states_t get_key_state(const int key);
		E_KEYSTATES get_key_state_raw(const int key);

		//bool handle_component(std::shared_ptr<components::c_component> component);

		void process_input(const int key, const E_KEYSTATES state, const float scroll);

		void process_input(const char character);

		inline bool m_tick_flag = false;

		inline std::vector<std::pair<std::string, custom_keybind_t>> m_active_keybinds = { };
	}
}

#endif