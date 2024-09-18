#include "c_tab_panel.hpp"

namespace rgui
{
	namespace elements
	{
		/*TAB NODES*/
		c_tab_node::c_tab_node(const std::string& title, const int index)
			: c_base_parent(title, { 0, 0 }, { 45, 20 }), c_base_icon('\0'), c_base_animated(math::easing::E_EASING::IN_OUT_CUBIC, 500, 350), c_base_openable(), c_indexed(index), c_base_callback()
		{

		}

		c_tab_node::c_tab_node(const std::string& title, const char icon, const int index)
			: c_base_parent(title, { 0, 0 }, { 45, 20 }), c_base_icon(icon), c_base_animated(math::easing::E_EASING::IN_OUT_CUBIC, 500, 350), c_base_openable(), c_indexed(index), c_base_callback()
		{

		}

		void c_tab_node::draw()
		{
			const auto& area = get_geometry_area();

			const auto& icon_size = m_style->get_font(E_STYLE_FONT::MEDIUM_ICON)->get_text_size(std::string({ m_icon }));

			const float blend = ease_input(!(get_is_selected() || get_is_open()));

			if (get_is_selected())
				drawing::draw_rectangle_filled({ static_cast<int>(area.position.x + icon_size.x * 1.5f - 5), area.position.y, area.points.right - icon_size.x - 10, area.points.bottom }, m_style->get_color(E_STYLE_COLOR::BACKGROUND_LIGHT), 5);

			auto icon_color = (get_is_selected() || get_is_child_selected()) ? m_style->get_color(E_STYLE_COLOR::ACCENT) : m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL).interpolate(m_style->get_color(E_STYLE_COLOR::ELEMENT_HIGHLIGHTED), blend);
			m_style->get_font(E_STYLE_FONT::MEDIUM_ICON)->draw({ area.position.x + 2, area.position.y + area.size.y * 0.5f }, icon_color, std::string({ m_icon }), drawing::TEXTFLAG_CENTERED_Y);
			if (m_icon == 'A')
			{
				icon_color.a *= 0.5f;
				m_style->get_font(E_STYLE_FONT::MEDIUM_ICON)->draw({ area.position.x + 2, area.position.y + area.size.y * 0.5f }, icon_color, std::string("B"), drawing::TEXTFLAG_CENTERED_Y);
			}

			m_style->get_font(E_STYLE_FONT::MEDIUM)->draw({ area.position.x + (icon_size.x > 0 ? 30 : 5), area.position.y + area.size.y * 0.5f }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL).interpolate(m_style->get_color(E_STYLE_COLOR::ELEMENT_HIGHLIGHTED), blend),get_title_formatted(), drawing::TEXTFLAG_CENTERED_Y);

			const std::size_t child_count = m_children.size();

			drawing::set_rotation(180.f * blend);

			if (child_count > 0)
			    m_style->get_font(E_STYLE_FONT::SMALL_ICON)->draw({ area.points.right - 24, area.position.y + area.size.y * 0.5f }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL).interpolate(m_style->get_color(E_STYLE_COLOR::ELEMENT_HIGHLIGHTED), blend), std::string("h"), drawing::TEXTFLAG_CENTERED_X | drawing::TEXTFLAG_CENTERED_Y);

			drawing::pop_rotation();

			if ((m_is_open || !m_input_animation.get_has_ended(m_input_smooth)) && child_count > 0)
			{
				const int height = (child_count + 1) * (area.size.y + 5) * blend;

				shapes::rect_t clip_area = { area.position, { m_parent->get_size().x - std::abs(area.position.x - m_parent->get_position().x), height } };
				const auto& backup_clip = drawing::get_clip();

				if (backup_clip.position.y > clip_area.position.y)
				{
					clip_area.size.y -= backup_clip.position.y - clip_area.position.y;
					clip_area.position.y = backup_clip.position.y;
					clip_area.update_points();
				}

				clip_area.size.y = clip_area.points.bottom > backup_clip.points.bottom ? backup_clip.points.bottom - clip_area.position.y : clip_area.size.y;
				clip_area.update_points();

				//if (scrollable)
				drawing::set_clip(clip_area);

				int line_selection_highlight_y = -1;
				math::point_t pad = { 26, m_size.y };
				for (auto& child : m_children)
				{
					child->set_position(m_position + pad);
					child->set_size({ m_size.x - pad.x, m_size.y });
					if (std::static_pointer_cast<c_tab_node>(child)->get_is_selected())
						line_selection_highlight_y = child->get_position().y;

					child->draw();
					pad.y += m_size.y + 5;
				}

				drawing::draw_rectangle_filled({ { area.position.x + icon_size.x * 0.5f + 2, area.position.y + area.size.y }, { 2, pad.y - m_size.y - 5 } }, m_style->get_color(E_STYLE_COLOR::BACKGROUND_LIGHT));

				if (line_selection_highlight_y > -1)
					drawing::draw_rectangle_filled({ { area.position.x + icon_size.x * 0.5f + 2, line_selection_highlight_y }, { 2, m_size.y } }, m_style->get_color(E_STYLE_COLOR::ACCENT));

				set_size({ m_size.x, std::max(m_size.y, height) });

				drawing::pop_clip();
			}
		}

		bool c_tab_node::process_input(const input::input_capture_t& input, const float scroll)
		{
			if (input.key == VK_LBUTTON && input.get_key_state().is_pressed())
			{
				const auto& icon_size = m_style->get_font(E_STYLE_FONT::MEDIUM_ICON)->get_text_size(std::string({ m_icon })), title_size = m_style->get_font(E_STYLE_FONT::SMALL)->get_text_size(get_title_formatted());

				const math::point_t text_area = { title_size.x + icon_size.x + 5, title_size.y };


				if (input::get_is_mouse_in_area({ { m_parent->get_position().x, get_position().y + get_size().y * 0.5f - text_area.y * 0.5f }, { m_parent->get_size().x, std::max(title_size.y, icon_size.y)}}))
				{
					if (m_input_animation.get_has_ended(m_input_smooth))
					{
						if (m_children.size() > 0)
							toggle_is_open();

						think_input();
					}

					return true;
				}
				else if (m_is_open && m_children.size() > 0)
				{
					for (auto child : m_children)
					{
						if (child->process_input(input, scroll))
						{
							set_value(std::static_pointer_cast<c_tab_node>(child)->get_index());
							std::static_pointer_cast<c_tab_node>(child)->run_input_callback();
							return true;
						}
					}
				}
			}

			return false;
		}

		void c_tab_node::update()
		{

		}

		bool c_tab_node::get_is_selected() const
		{
			return m_parent ? std::dynamic_pointer_cast<c_indexed>(m_parent)->get_selected_index() == get_index() && (!m_is_enabled_callback || m_is_enabled_callback()) : false;
		}

		bool c_tab_node::get_is_child_selected() const
		{
			for (const auto& child : m_children)
				if (std::static_pointer_cast<c_tab_node>(child)->get_is_selected())
					return true;

			return false;
		}

		math::point_t c_tab_node::get_children_size() const
		{
			return { get_size().x, ((get_size().y + 5) * m_children.size()) * ease_input(!(get_is_selected() || get_is_open())) };
		}

		/*TAB PANEL*/
		c_tab_panel::c_tab_panel()
			: c_base_parent(), c_base_callback(), m_node_size({ }), m_node_margin(0), m_spacing_top(0)
		{

		}

		c_tab_panel::c_tab_panel(const std::string& title, const math::point_t& position, const math::point_t& size, const math::point_t& node_size, const int spacing_top)
			: c_base_parent(title, position, size), c_base_callback(), m_node_size(node_size), m_node_margin(22), m_spacing_top(spacing_top)
		{
			push_flag(E_ELEMENT_FLAG::NO_TARGET);
		}

		void c_tab_panel::draw()
		{
			const auto& area = get_geometry_area();
			drawing::draw_rectangle_filled(area, m_style->get_color(E_STYLE_COLOR::BACKGROUND_DARK), m_rounding);
			drawing::draw_rectangle_filled({ area.position + math::point_t(m_rounding, 0), area.size - math::point_t(m_rounding, 0) }, m_style->get_color(E_STYLE_COLOR::BACKGROUND_DARK));

			drawing::set_clip(area);

			int pad = m_spacing_top - m_scroll;
			for (auto child : m_children)
			{
				child->set_position({ m_position.x + m_node_margin * std::min(1.f, m_scale), m_position.y + pad });
				child->set_size({ m_size.x - m_node_margin * std::min(1.f, m_scale), 36 });
				child->draw();
				const auto& child_size = child->get_size(true);

				pad += child_size.y;

				child->set_size({ m_size.x, 36 }); //draw() sets the size to the sum of all the sub-children together
			}

			drawing::pop_clip();
		}

		bool c_tab_panel::process_input(const input::input_capture_t& input, const float scroll)
		{
			if (get_is_scrollable() && input::get_is_mouse_in_area(get_geometry_area()) && scroll != 0.f)
			{
				const int backup_scroll = m_scroll;

				int max_scroll = m_spacing_top;

				for (auto& child : m_children)
					max_scroll += child->get_size().y + std::static_pointer_cast<c_tab_node>(child)->get_children_size().y;

				process_scroll(scroll, m_size, { m_size.x, max_scroll });

				return true;
			}
			else
			{
				for (auto& child : m_children)
				{
					if (child->process_input(input, scroll))
					{
						auto child_n = std::static_pointer_cast<c_tab_node>(child);

						if (child_n->get_children().size() <= 0)
							set_value(child_n->get_index());

						run_input_callback();

						return true; //true
					}
				}
			}

			return false;
		}

		void c_tab_panel::update()
		{

		}
	}
}