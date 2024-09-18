#pragma once
#ifndef C_BASE_DRAGGABLE_HPP
#define C_BASE_DRAGGABLE_HPP
#include "../../helpers/input/input.hpp"

namespace rgui
{
	namespace elements
	{
		class c_base_draggable
		{
		public:
			c_base_draggable()
				: m_is_being_dragged(false), m_drag_begin_point({ })
			{

			}

		public:
			inline bool set_is_being_dragged(const bool flag)
			{
				if (!m_currently_dragged || m_currently_dragged == this)
				{
					if (!m_is_being_dragged && flag) //just started dragging
					{
						m_is_being_dragged = true;
						m_currently_dragged = this;
						m_drag_begin_point = input::get_mouse_pos();
						return true;
					}
					else if (!flag) //stopped dragging
					{
						m_is_being_dragged = false;
						m_currently_dragged = nullptr;
						m_drag_begin_point = { };
					}
				}
				else 
					m_is_being_dragged = false;

				return false;
			}

			inline bool get_is_being_dragged() const
			{
				return m_is_being_dragged;
			}

			inline void set_drag_begin_point(const math::point_t& point)
			{
				m_drag_begin_point = point;
			}

			inline math::point_t get_drag_begin_point() const
			{
				return m_drag_begin_point;
			}


		protected:
			bool m_is_being_dragged;
			math::point_t m_drag_begin_point;

		public:
			inline static c_base_draggable* get_currently_dragged()
			{
				return m_currently_dragged;
			}

		private:
			inline static c_base_draggable* m_currently_dragged;
		};
	}
}
#endif
