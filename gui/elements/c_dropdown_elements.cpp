#include "c_dropdown_elements.hpp"
#include "c_checkbox.hpp"
#include "c_colorpicker.hpp"

namespace rgui
{
	namespace elements
	{
		c_dropdown_elements::c_dropdown_elements(const std::string& title, const int child_panel_child, const math::point_t& position, const math::point_t& size)
			: c_base_parent(title, position, size, math::point_t(5, 5)), c_base_openable(), c_base_animated(math::easing::E_EASING::OUT_SINE, 400, 200), m_child_panel_height(child_panel_child)
		{
			pop_flag(E_ELEMENT_FLAG::NO_TARGET);
		}

		void c_dropdown_elements::draw()
		{
			const shapes::rect_t& geometry_area = get_geometry_area();
			drawing::draw_rectangle_filled(geometry_area, m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND), 7);
			drawing::draw_rectangle(geometry_area, m_style->get_color(E_STYLE_COLOR::GENERAL_OUTLINE), 7);

			//m_style->get_font(E_STYLE_FONT::SMALL_ICON)->draw({ geometry_area.points.right - 5, geometry_area.points.top + geometry_area.size.y / 2 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL), { m_icon }, drawing::TEXTFLAG_LEFT | drawing::TEXTFLAG_CENTERED_Y);

			int line_length = 8, line_height = geometry_area.size.y / 2 - 3;
			for (std::size_t i = 0; i < 4; i++)
			{
				drawing::draw_line({ geometry_area.points.right - line_length - 7, geometry_area.position.y + line_height }, { geometry_area.points.right - 7, geometry_area.position.y + line_height }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL));
				line_height += 2;
			}

			auto small_font = m_style->get_font(E_STYLE_FONT::SMALL);

			small_font->draw({ geometry_area.position.x, geometry_area.position.y - 2 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL), get_title_formatted(), drawing::TEXTFLAG_BOTTOM);

			const math::point_t child_panel_position = { geometry_area.position.x, geometry_area.position.y + geometry_area.size.y + 6 };
			shapes::rect_t displayable_options_area = { child_panel_position, { geometry_area.size.x, m_child_panel_height } };//, complete_options_area = { child_panel_position, get_children_size() };

			//const bool clip = complete_options_area.size.y > displayable_options_area.size.y;

			const bool has_animation_ended = m_input_animation.get_has_ended(200);
			bool run_hover_animation = input::get_is_mouse_in_area(geometry_area);
			if (m_is_open || !has_animation_ended)
			{
				draw_input_fill_animation(geometry_area, m_style->get_color(E_STYLE_COLOR::GENERAL_OUTLINE), ease_input());

				run_hover_animation = false; //now reverse the animation because it's already opened

				displayable_options_area.size.y *= ease_input(!m_is_open);
				displayable_options_area.update_points();

				drawing::draw_rectangle_filled(displayable_options_area, m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND), 7);
				drawing::draw_rectangle_filled(displayable_options_area, m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND), 7);
				drawing::draw_rectangle(displayable_options_area, m_style->get_color(E_STYLE_COLOR::GENERAL_OUTLINE), 7);

				drawing::set_clip({ { geometry_area.position.x, geometry_area.position.y + geometry_area.size.y + 1 }, { displayable_options_area.size.x, displayable_options_area.size.y - 1 } });

				/*int padding = 5;
				for (std::size_t i = 0; i < m_children.size(); i++)
				{
					auto& child = m_children.at(i);
					const math::point_t& child_position = child->get_position(), child_size = child->get_size();

					if (child_position.y + child_size.y > displayable_options_area.points.bottom)
						break; //sorry
					else if (child_position.y > displayable_options_area.points.top)
						

					padding += child_size.y;
				}*/

				c_base_parent::draw();

				drawing::pop_clip();
			}

			//hover outline
			{
				const float blend_hover = think_hover(run_hover_animation);
				drawing::color_t outline_color = m_style->get_color(E_STYLE_COLOR::ACCENT);
				outline_color.a *= blend_hover;
				drawing::draw_rectangle(geometry_area, outline_color, 7);
			}

			//draw scrollbar
			if (m_is_open && has_animation_ended)
			{
				const int area_delta = get_children_size().y - displayable_options_area.size.y, scroll_bar_padding = 6;
				drawing::draw_rectangle_filled({ { displayable_options_area.points.right - scroll_bar_padding - 4, displayable_options_area.points.top + scroll_bar_padding }, { 4, displayable_options_area.size.y - (scroll_bar_padding * 2) /*times two because there's padding on top so I have to account for that aswell*/ } }, m_style->get_color(E_STYLE_COLOR::SCROLLBAR_BACKRGROUND), 7);
				drawing::draw_rectangle_filled({ { displayable_options_area.points.right - scroll_bar_padding - 4, displayable_options_area.points.top + static_cast<int>(m_scroll) + scroll_bar_padding }, { 4, (displayable_options_area.size.y - area_delta) - (scroll_bar_padding * 2) /*times two because there's padding on top so I have to account for that aswel*/ } }, m_style->get_color(E_STYLE_COLOR::SCROLLBAR_FOREGROUND), 7);
			}
		}

		bool c_dropdown_elements::process_input(const input::input_capture_t& input, const float scroll)
		{
			const auto& geometry_area = get_geometry_area();

			auto small_font = m_style->get_font(E_STYLE_FONT::SMALL);

			//Hi, note: to fix the thingies not opening make a target element in every c_base_parent thx.
			if (m_is_open && c_base_parent::process_input(input, scroll))
				return true;
			else
			{
				//loop through children and send input if nothing happened then process input on the dropdown
				if (input.key == VK_LBUTTON && input.get_key_state().is_pressed())
				{
					if (input::get_is_mouse_in_area(get_geometry_area()))
					{
						think_input();
						toggle_is_open();

						return true;
					}
					else if (m_is_open)
					{
						if (input::get_is_mouse_in_area({ { geometry_area.position.x,  geometry_area.position.y + geometry_area.size.y }, { geometry_area.size.x, m_child_panel_height } }))
							return true;
						else
						{
							if (m_is_open)
							{
								m_is_open = false;
								think_input();
								return false; //return false so that other things are clickable
							}
						}
					}
				}
			}

			return false;
		}

		void c_dropdown_elements::update()
		{

		}

		math::point_t c_dropdown_elements::calculate_auto_position(const int space_between, const bool sameline) const
		{
			return { m_spacing.x, get_previous_child_height(sameline) + (m_children.empty() ? get_size().y : space_between) + 2 };
		}

		std::shared_ptr<c_checkbox> c_dropdown_elements::checkbox(const std::string& title, bool* variable, const int space_between)
		{
			auto checkbox = c_base_parent::checkbox(title, variable, space_between);
			const math::point_t& position = checkbox->get_position();
			checkbox->set_position({ position.x + m_spacing.x + 4, position.y });
			return checkbox;
		}

		std::shared_ptr<c_colorpicker> c_dropdown_elements::colorpicker(const std::string& title, drawing::color_t* variable, const bool draw_title, const bool same_line, const int space_between, const int margin)
		{
			auto colorpicker = c_base_parent::colorpicker(title, variable, draw_title, space_between);
			const math::point_t& position = colorpicker->get_position();
			colorpicker->set_position({ position.x - 6, position.y });
			return colorpicker;
		}
	}
}
