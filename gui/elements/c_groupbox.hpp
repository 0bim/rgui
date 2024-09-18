#pragma once
#ifndef C_GROUPBOX_HPP
#define C_GROUPBOX_HPP

#include "base/c_base_parent.hpp"
#include "base/c_base_icon.hpp"

namespace rgui
{
	namespace elements
	{
		class c_groupbox : public c_base_parent, public c_base_icon
		{
		public:
			c_groupbox(const std::string& title, const math::point_t& position, const math::point_t& size, const char icon = -1);

			c_groupbox(const std::string& title, const math::point_t& position, const char icon = -1);

		public:
			void draw() override;

			bool process_input(const input::input_capture_t& input, const float scroll) override;

			void update() override;

		public:
			inline void set_auto_resize(const bool flag)
			{
				m_auto_resize = flag;
			}

			inline bool get_auto_resize() const
			{
				return m_auto_resize;
			}

			inline void push_child(const std::shared_ptr<c_base_element>& child) override final
			{
				c_base_parent::push_child(child);
				if (get_auto_resize())
					force_auto_resize();
			}

			inline void force_auto_resize()
			{
				m_size.y = get_children_size().y + m_spacing.y * 0.5f;
			}

		private:
			bool m_auto_resize;
		};
	}
}

#endif
