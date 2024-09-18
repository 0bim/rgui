#pragma once
#ifndef C_BASE_PARENT_HPP
#define C_BASE_PARENT_HPP
#include "c_base_element.hpp"
#include "c_base_scrollable.hpp"
#include "../c_slider.hpp"
#include "../c_input_box.hpp"

namespace rgui
{
	namespace elements
	{
		class c_button;
		class c_checkbox;
		class c_switchbox;
		class c_colorpicker;
		class c_dropdown;
		class c_dropdown_elements;
		class c_keybinder;
		class c_list_box;
		class c_text_box;
		class c_text;
		class c_groupbox;

		class c_base_parent : public c_base_element, public c_base_scrollable
		{
		public:
			c_base_parent()
				: c_base_element(), c_base_scrollable(), m_children(), m_target_child(nullptr), m_spacing({ 0, 0 }), m_extra_space(0)
			{
				set_port_area({ m_size.x - m_spacing.x, m_size.y });
			}

			c_base_parent(const std::string& title, const math::point_t& position, const math::point_t& size, const math::point_t& spacing = { 25, 10 })
				: c_base_element(title, position, size), c_base_scrollable(), m_children(), m_target_child(nullptr), m_spacing(spacing), m_extra_space(0)
			{
				set_port_area({ m_size.x - m_spacing.x, m_size.y });
			}

		public:
			/*this is a really ugly hack but idk how to yes.*/
			inline virtual void draw() override //or paint idk what to call it :/
			{
				const auto& area = get_geometry_area();

				for (auto& child : m_children)
					if (child != m_target_child)
						drawing::draw_element(child);

				drawing::draw_element(m_target_child);

				const bool scrollable = get_is_scrollable() && get_children_size().y > get_size().y;

				if (scrollable)
				{
					auto start_color = m_style->get_color(E_STYLE_COLOR::BACKGROUND_LIGHT), end_color = m_style->get_color(E_STYLE_COLOR::BACKGROUND_LIGHT);
					start_color.a = 0.f;
					end_color.a *= 0.95f;
					const float ratio_bottom = m_scroll / (get_max_vertical_scroll() - get_size().y);
					drawing::draw_rectangle_filled({ { area.position.x, area.points.bottom - area.size.y * 0.22f * (1.f - ratio_bottom) }, { area.size.x, area.size.y * 0.22f } }, start_color, end_color, false, m_rounding);
					if (m_scroll != 0)
						drawing::draw_rectangle_filled({ area.position , { area.size.x, area.size.y * 0.22f * ratio_bottom } }, end_color, start_color, false, m_rounding);
				}

				/*int new_scroll = std::clamp<int>(m_scroll, 0, std::max<int>(0, get_max_vertical_scroll() - area.size.y));
				if (new_scroll != m_scroll)
					process_scroll(m_scroll, get_size(), { get_max_horizontal_scroll(), get_max_vertical_scroll() });*/
			}

			inline virtual bool process_input(const input::input_capture_t& input, const float scroll) override
			{
				if (get_is_visible())
				{
					if (m_target_child && m_target_child->get_is_visible() && m_target_child->get_is_input_enabled())
					{
						if (m_target_child->process_input(input, scroll))
							return true;

						if (m_target_child->m_destruct_post_use)
						{
							m_children.erase(std::remove(m_children.begin(), m_children.end(), m_target_child));
							m_target_child = nullptr;
						}
					}

					//would use auto& child : m_children but it causes issues with manual map
					for (std::size_t i = 0; i < m_children.size(); i++)
					{
						const auto& child = m_children[i];
						if (child && child != m_target_child && child->get_is_input_enabled() && child->get_is_visible())
						{
							if (child->process_input(input, scroll))
							{
								set_target_child(child);
								return true; //STOP INSTANTLY!!!
							}
						}
					}

					if (get_is_scrollable() && scroll != 0.f && get_children_size().y > get_size().y && input::get_is_mouse_in_area(get_geometry_area()))
					{
						const int backup_scroll = m_scroll; //backup the old scroll amount

						process_scroll(scroll, get_size(), { get_max_horizontal_scroll(), get_max_vertical_scroll() }); //process new scroll

						for (const auto& child : m_children)
							if (child && !child->get_ignore_scroll())
							    child->on_move({ 0, backup_scroll - m_scroll /*remove old scroll and replace with new and add padding*/ });

						return true;
					}
				}

				return false;
			}

			inline virtual bool process_input(const char character) override
			{
				if (m_target_child && m_target_child->get_is_input_enabled() && m_target_child->process_input(character))
					return true;

				//would use auto& child : m_children but it causes issues with manual map
				for (std::size_t i = 0; i < m_children.size(); i++)
				{
					const auto& child = m_children[i];
					if (child != m_target_child && child->get_is_input_enabled() && child->get_is_visible() && child->process_input(character))
					{
						set_target_child(child);
						return true; /*STOP INSTANTLY!!!*/
					}
				}

				return false;
			}

			inline virtual void update() override
			{
				for (auto& child : m_children)
					if (child)
					    child->update();
			}

			inline void on_move(const math::point_t& delta) override
			{
				m_position += delta;
				for (auto& child : m_children)
					if (child)
					    child->on_move(delta);
			}

			inline virtual void draw_scrollbar(const shapes::rect_t& shaft_area, const float rounding, const bool horizontal)
			{
				drawing::draw_scrollbar(get_size().y, get_max_vertical_scroll(), m_scroll, shaft_area, rounding, horizontal);
			}

		public:
			int get_previous_child_height(const bool sameline = false) const;

			virtual math::point_t calculate_auto_position(const int space_between, const bool sameline = false) const;

			std::shared_ptr<c_button> button(const std::string& title, const math::point_t& size, const int space_between = 10);

			virtual std::shared_ptr<c_checkbox> checkbox(const std::string& title, bool* variable = nullptr, const int space_between = 10);

			std::shared_ptr<c_switchbox> switchbox(const std::string& title, bool* variable = nullptr, const int space_between = 15);

			virtual std::shared_ptr<c_colorpicker> colorpicker(const std::string& title, drawing::color_t* variable = nullptr, const bool draw_title = false, const bool same_line = true, const int space_between = 0, const int margin = 20);

			std::shared_ptr<c_dropdown> dropdown(const std::string& title, const int space_between = 35);

			std::shared_ptr<c_dropdown_elements> dropdown_elements(const std::string& title, const int child_panel_height, const int space_between = 35);

			std::shared_ptr<c_list_box> list_box(const std::string& title, const int height, const int space_between = 35);

			std::shared_ptr<c_keybinder> keybinder(const std::string& title, input::custom_keybind_t* variable = nullptr, const int space_between = 0, const int margin = 12);

			template<typename T>
			std::shared_ptr<c_slider<T>> slider(const std::string& title, T* variable = nullptr, const int space_between = 40);

			std::shared_ptr<c_text_box> text_box(const std::string& title, std::string* variable = nullptr, const int space_between = 30);

			template<input_number_t T>
			std::shared_ptr<c_input_box<T>> input_box(const std::string& title, T* variable = nullptr, const int space_between = 25);

			std::shared_ptr<c_text> text(const std::string& title, const math::point_t& position, drawing::font_t* font, const E_STYLE_COLOR color_index, const std::uint8_t text_flags, const int space_between = 20);

			std::shared_ptr<c_groupbox> groupbox(const std::string& title, const math::point_t& position, const char icon = -1);

		public:
			inline virtual void push_child(const std::shared_ptr<c_base_element>& child)
			{
				m_extra_space = 0;
				child->set_position(child->get_position() + m_position);
				child->set_parent(std::static_pointer_cast<c_base_parent>(shared_from_this()));
				child->set_port_area({ m_size.x - m_spacing.x * 2.f, m_size.y });
				m_children.push_back(child);
			}

			inline void push_child(const std::vector<std::shared_ptr<c_base_element>>& children)
			{
				for (auto& child : children)
					push_child(child);
			}

			inline virtual void remove_child(const std::shared_ptr<c_base_element>& child)
			{
				auto it = std::find(m_children.begin(), m_children.end(), child);
				if (it != m_children.end())
					m_children.erase(it);
			}

			inline std::vector<std::shared_ptr<c_base_element>>& get_children()
			{
				return m_children;
			}

			inline virtual math::point_t get_children_size() const
			{
				math::point_t size = { 0, 0 };

				for (const auto& child : m_children)
				{
					if (!child->get_is_visible())
						continue;

					const math::point_t& child_position = child->get_position(), child_size = child->get_size();
					size.x = std::max(size.x, child_position.x + child_size.x);
					size.y = std::max(size.y, child_position.y + child_size.y + m_scroll);
				}

				//because m_position is added to all children it needs to be subtracted here to get the correct size
				return size - m_position;
			}

			inline void set_target_child(const std::shared_ptr<c_base_element>& child)
			{
				if (!child || (!child->get_flag(E_ELEMENT_FLAG::NO_TARGET) && m_target_child != child))
				{
					if (m_target_child)
						m_target_child->on_retarget();
					m_target_child = child;
				}
			}

			inline std::shared_ptr<c_base_element> get_target_child()
			{
				return m_target_child;
			}

			inline void set_spacing(const math::point_t& spacing)
			{
				m_spacing = spacing;
			}

			inline math::point_t get_spacing() const
			{
				return m_spacing;
			}

			inline virtual float get_max_vertical_scroll()
			{
				const auto occupied = get_children_size().y;
				return occupied + (occupied > get_size().y ? m_spacing.x : 0);
			}

			inline virtual float get_max_horizontal_scroll()
			{
				const auto occupied = get_children_size().x;
				return occupied + (occupied > get_size().x ? m_spacing.x : 0);
			}

			inline void reset_scroll()
			{
				//process_scroll(-m_scroll, get_size(), { get_max_horizontal_scroll(), get_max_vertical_scroll() }); //process new scroll

				for (const auto& child : m_children)
					child->on_move({ 0, m_scroll });

				m_scroll = 0;
			}

			inline std::shared_ptr<c_base_element> find_child_traverse(const std::string& title) override
			{
				for (const auto& child : m_children)
					if (const auto& c = child->find_child_traverse(title))
						return c;

				return nullptr;
			}

		protected:
			std::vector<std::shared_ptr<c_base_element>> m_children;

			std::shared_ptr<c_base_element> m_target_child;

			math::point_t m_spacing;

			int m_extra_space;
		};
	}
}
#endif