#pragma once
#ifndef C_BASE_VALUE_CHANGE_CALLBACK_HPP
#define C_BASE_VALUE_CHANGE_CALLBACK_HPP
#include <functional>

namespace rgui
{
	namespace elements
	{
		template<typename T>
		class c_base_value_change_callback
		{
		public:
			c_base_value_change_callback()
				: m_value_change_callback(nullptr)
			{

			}
			
		public:
			inline void set_value_change_callback(const std::function<void(const T value)>& callback)
			{
				m_value_change_callback = callback;
			}

			inline std::function<void(const T value)> get_value_change_callback()
			{
				return m_value_change_callback;
			}

			inline void run_value_change_callback(const T value)
			{
				if (m_value_change_callback)
					m_value_change_callback(value);
			}

		protected:
			std::function<void(const T value)> m_value_change_callback;
		};
	}
}

#endif