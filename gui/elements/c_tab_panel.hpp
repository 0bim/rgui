#pragma once
#ifndef C_TAB_PANEL_HPP
#define C_TAB_PANEL_HPP
#include "base/c_base_element.hpp"
#include "base/c_base_parent.hpp"
#include "base/c_base_icon.hpp"
#include "base/c_base_animated.hpp"
#include "base/c_base_variable_controller.hpp"
#include "base/c_base_callback.hpp"

namespace rgui
{
	namespace elements
	{
		class c_indexed : public c_base_variable_controller<int>
		{
		public:
			c_indexed(const int index = -1)
				: c_base_variable_controller(), m_index(index)
			{

			}

			inline void set_index(const int index)
			{
				m_index = index;
			}

			inline int get_index() const
			{
				return m_index;
			}

			inline void set_selected_index(const int index)
			{
				set_value(index);
			}

			inline int get_selected_index() const
			{
				return get_value();
			}

		protected:
			int m_index;
		};

		/*TAB NODES*/
		class c_tab_node : public c_base_parent, public c_base_openable, public c_base_animated, public c_base_icon, public c_indexed, public c_base_callback
		{
		public:
			c_tab_node(const std::string& title, const int index);

			c_tab_node(const std::string& title, const char icon, const int index);

			void draw() override final;

			bool process_input(const input::input_capture_t& input, const float scroll) override final;

			void update() override final;
			
			bool get_is_selected() const;

			bool get_is_child_selected() const;

			math::point_t get_children_size() const;

		public:
			inline void remove_child(const std::shared_ptr<c_base_element>& child) override final
			{
				c_base_parent::remove_child(child);
				if (m_children.empty())
					set_is_open(false);
			}
		};

		/*TAB PANEL*/
		class c_tab_panel : public c_base_parent, public c_base_callback, public c_indexed
		{
		public:
			c_tab_panel();

			c_tab_panel(const std::string& title, const math::point_t& position, const math::point_t& size, const math::point_t& node_size, const int spacing_top);

			void draw() override final;

			bool process_input(const input::input_capture_t& input, const float scroll) override final;

			void update() override final;

		private:
			math::point_t m_node_size;
			int m_node_margin;
			int m_spacing_top;
		};
	}
}

#endif