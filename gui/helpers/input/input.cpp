#include "input.hpp"
#include "../../drawing/render/render.hpp"
#include "../../elements/base/c_base_form.hpp"

namespace rgui
{
	namespace input
	{
		inline std::array<E_KEYSTATES, 512> m_key_states = { };
		
		inline math::point_t m_mouse_pos = { 0, 0 };

		inline char m_last_char = '\0';

		bool get_is_mouse_in_area(const shapes::rect_t& rect)
		{
			const math::point_t& size = rect.size;
			const math::point_t& position = rect.position + size * 0.5f; //get to the middle
			const math::point_t& delta = (position - m_mouse_pos);
			return (delta.abs() < math::point_t(size.x * 0.5f + 1, size.y * 0.5f));
		}

		/*bool get_is_component_hovered(std::shared_ptr<components::c_component> component)
		{
			static std::shared_ptr<components::c_component> previous_component = component;
			static bool previous_flag = false;

			math::vector2d pos = component->get_pos();
			math::vector2d size = component->get_size();

			bool flag = get_is_mouse_in_area({ pos, size });

			if (component != previous_component && flag && previous_flag)
				return false;

			previous_component = component;
			previous_flag = flag;

			//if (flag)
			//	size.x += 1;

			return flag;
		}*/

		void set_mouse_pos(const math::point_t& pos)
		{
			m_mouse_pos = pos;
		}

		math::point_t get_mouse_pos()
		{
			return m_mouse_pos;
		}
		
		key_states_t get_key_state(const int key)
		{
			return m_key_states[key];
		}

		E_KEYSTATES get_key_state_raw(const int key)
		{
			return m_key_states.at(key);
		}

		void set_key_state(const int key, const E_KEYSTATES state)
		{
			m_key_states[key] = state;
		}

		/*bool handle_component(std::shared_ptr<components::c_component> component)
		{
			if (!m_tick_flag)
				return false;

			for (input_capture_t accepted_input : component->get_accepted_input())
				if (get_key_state_raw(accepted_input.key) == accepted_input.state)
					if (component->handle_input())
					{
						m_tick_flag = false;
						return true;
					}
			return false;
		}*/

		void process_input(const int key, const E_KEYSTATES state, const float scroll)
		{
			if ((/*state == E_KEYSTATES::NONE ||*/ key > 256 || key < 0) && scroll == 0.f)
				return;

			const input_capture_t key_state = { static_cast<std::size_t>(key), state };

			if (drawing::m_target_form && drawing::m_target_form->get_is_input_enabled() && drawing::m_target_form->get_is_visible() && drawing::m_target_form->process_input(key_state, scroll))
				return;

			for (const auto& form : drawing::get_forms())
				if (form && form != drawing::m_target_form && form->get_is_input_enabled() && form->get_is_visible() && form->process_input(key_state, scroll))
				{
					drawing::m_target_form = form;
					return;
				}
		}

		void process_input(const char character)
		{
			if (!character)
				return;

			if (drawing::m_target_form && drawing::m_target_form->get_is_input_enabled() && drawing::m_target_form->get_is_visible() && drawing::m_target_form->process_input(character))
				return;

			for (const auto& form : drawing::get_forms())
				if (form && form != drawing::m_target_form && form->get_is_input_enabled() && form->get_is_visible() && form->process_input(character))
				{
					drawing::m_target_form = form;
					return;
				}
		}
	}
}