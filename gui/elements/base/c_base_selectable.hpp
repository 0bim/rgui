#pragma once
#ifndef C_BASE_SELECTABLE
#define C_BASE_SELECTABLE
#include "c_base_index_callback.hpp"

namespace rgui
{
	namespace elements
	{
		class c_base_selectable : public c_base_index_callback
		{
		public:
			c_base_selectable() 
				: c_base_index_callback(), m_filter("")
			{
				m_options = { };
				m_options.clear();
			}

		public:
			inline void set_options(const std::vector<std::string>& options)
			{
				m_options.clear();
				m_options = options;
				on_options_resize();
			}

			inline void push_option(const std::string& option)
			{
				m_options.push_back(option);
				on_options_resize();
			}

			inline virtual void on_options_resize()
			{
				//to override :D
            }

			inline const std::vector<std::string>& get_options() const
			{
				return m_options;
			}

			inline std::vector<std::string> get_options()
			{
				return m_options;
			}

			inline void set_filter(const std::string& filter)
			{
				m_filter = filter;
			}

			inline const std::string& get_filter() const
			{
				return m_filter;
			}

		protected:
			std::vector<std::string> m_options;

			std::string m_filter;
		};
	}
}

#endif