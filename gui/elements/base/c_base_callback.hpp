#pragma once
#ifndef C_BASE_CALLBACK
#define C_BASE_CALLBACK
#include <functional>

namespace rgui
{
	namespace elements
	{
		class c_base_callback
		{
		public:
			c_base_callback() : m_input_callback({ }), m_is_enabled_callback({ })
			{

			}

			inline void set_input_callback(const std::function<void()>& callback)
			{
				m_input_callback = callback;
			}

			inline std::function<void()> get_input_callback() const
			{
				return m_input_callback;
			}

			inline void set_is_enabled_callback(const std::function<bool()> callback)
			{
				m_is_enabled_callback = callback;
			}

			inline std::function<bool()> get_is_enabled_callback() const
			{
				return m_is_enabled_callback;
			}

			inline bool get_is_enabled()
			{
				if (m_is_enabled_callback)
					return m_is_enabled_callback();

				return false;
			}

		protected:
			inline void run_input_callback()
			{
				if (m_input_callback)
					m_input_callback();
			}

			std::function<void()> m_input_callback;
			std::function<bool()> m_is_enabled_callback;
		};
	}
}

#endif