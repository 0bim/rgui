#pragma once
#ifndef C_BASE_VARIABLE_CONTROLLER
#define C_BASE_VARIABLE_CONTROLLER

namespace rgui
{
	namespace elements
	{
		template<typename T>
		class c_base_variable_controller
		{
		public:
			c_base_variable_controller()
				: m_variable(nullptr)
			{

			}

		public:
			inline virtual void set_variable(T* variable)
			{
				m_variable = variable;
			}

			inline T* get_variable() const
			{
				return m_variable;
			}

			inline virtual void set_value(T value)
			{
				if (m_variable)
				   *m_variable = value;
			}

			inline T get_value() const
			{
				return (get_is_valid() ? (*get_variable()) : std::numeric_limits<T>::min());
			}

			inline bool get_is_valid() const
			{
				return !!m_variable;
			}

		protected:
			T* m_variable;
		};
	}
}

#endif