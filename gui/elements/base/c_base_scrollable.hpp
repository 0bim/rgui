#pragma once
#ifndef C_BASE_SCROLLABLE_HPP
#define C_BASE_SCROLLABLE_HPP
#include "../../helpers/math/math.hpp"

namespace rgui
{
	namespace elements
	{
		class c_base_scrollable
		{
		public:
			c_base_scrollable() 
				: m_scroll(0), m_is_scrollable(true)
			{

			}

		public:
			inline void set_scroll(const int scroll)
			{
				m_scroll = scroll;
			}

			inline void append_scroll(const int scroll)
			{
				m_scroll += scroll;
			}

			inline int get_scroll() const
			{
				return m_scroll;
			}

			inline void set_is_scrollable(const bool is_scrollable)
			{
				m_is_scrollable = is_scrollable;
			}

			inline bool get_is_scrollable() const
			{
				return m_is_scrollable;
			}

			inline virtual void process_scroll(const float scroll, const math::point_t& displayable_size, const math::point_t& complete_size)
			{
				if (get_is_scrollable() && complete_size.y > displayable_size.y)
				{
					m_scroll += scroll;
					m_scroll = std::clamp(m_scroll, 0, complete_size.y - displayable_size.y);
				}
			}

		protected:
			bool m_is_scrollable;

			int m_scroll;
		};
	}
}

#endif