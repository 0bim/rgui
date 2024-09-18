#pragma once
#ifndef C_BASE_INDEX_CALLBACK
#define C_BASE_INDEX_CALLBACK
#include <functional>

namespace rgui
{
	namespace elements
	{
		class c_base_index_callback
		{
		public:
			c_base_index_callback() 
				: m_input_callback(nullptr), m_is_enabled_callback(nullptr)
			{

			}

			inline void set_input_callback(const std::function<void(const std::size_t index)> callback)
			{
				m_input_callback = callback;
			}

			inline std::function<void(const std::size_t index)> get_input_callback() const
			{
				return m_input_callback;
			}

			inline void set_is_selected_callback(const std::function<bool(const std::size_t index)> callback)
			{
				m_is_selected_callback = callback;
			}

			inline std::function<bool(const std::size_t index)> get_is_selected_callback() const
			{
				return m_is_selected_callback;
			}

			inline bool get_is_selected(const std::size_t index)
			{
				if (m_is_selected_callback)
					return m_is_selected_callback(index);

				return false;
			}

			inline void set_is_enabled_callback(const std::function<bool(const std::size_t index)> callback)
			{
				m_is_enabled_callback = callback;
			}

			inline std::function<bool(const std::size_t index)> get_is_enabled_callback() const
			{
				return m_is_enabled_callback;
			}

			inline bool get_is_enabled(const std::size_t index)
			{
				if (m_is_enabled_callback)
					return m_is_enabled_callback(index);

				return false;
			}

		protected:
			inline void run_input_callback(const std::size_t index)
			{
				if (m_input_callback)
					m_input_callback(index);
			}

			std::function<void(const std::size_t index)> m_input_callback;
			std::function<bool(const std::size_t index)> m_is_selected_callback, m_is_enabled_callback;
		};
	}
}

#endif