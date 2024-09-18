#include "c_list_box.hpp"

namespace rgui
{
	namespace elements
	{
		c_list_box::c_list_box(const std::string& title, const math::point_t& position, const math::point_t& size)
			: c_base_element(title, position, size), c_base_selectable(), c_base_animated(math::easing::E_EASING::OUT_SINE, 400, 200), c_base_scrollable()
		{

		}

		void c_list_box::draw()
		{
			const auto& geometry_area = get_geometry_area();

			m_style->get_font(E_STYLE_FONT::SMALL)->draw(geometry_area.position - math::point_t(0, 2), m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL),get_title_formatted(), drawing::TEXTFLAG_BOTTOM);

			drawing::draw_rectangle_filled(geometry_area, m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND), m_rounding);

			const auto& complete_options_area = get_options_area(true, true), displayable_options_area = get_options_area(false, true);

			shapes::rect_t clip_area = { geometry_area.position, geometry_area.size - math::point_t(8, 0) };
			const auto& backup_clip = drawing::get_clip();

			if (backup_clip.position.y > clip_area.position.y)
			{
				clip_area.size.y -= backup_clip.position.y - clip_area.position.y;
				clip_area.position.y = backup_clip.position.y;
				clip_area.update_points();
			}

			clip_area.size.y = clip_area.points.bottom > backup_clip.points.bottom ? backup_clip.points.bottom - clip_area.position.y : clip_area.size.y;
			clip_area.update_points();

			drawing::set_clip(clip_area);

			constexpr int padding = 5;
			math::point_t position = { geometry_area.position.x + 5, geometry_area.position.y + padding - m_scroll };
			bool stop_drawing = false;
			for (std::size_t i = 0; i < m_options.size(); i++)
			{
				if (m_options.at(i).find(m_filter) == std::string::npos)
					continue;

				if (position.y > displayable_options_area.points.bottom)
					break; //sorry
				else if (position.y + get_option_size(i).y > displayable_options_area.points.top)
				{
					draw_option(i, position);
					/*if (get_is_enabled(i))
					{
						const math::point_t checkmark_size = { 10, 10 };
						drawing::draw_checkmark({ displayable_options_area.points.right - checkmark_size.x - 12, position.y }, checkmark_size, m_style->get_color(E_STYLE_COLOR::ACCENT));
					}*/
				}

				position.y += get_option_size(i).y + padding;
			}

			drawing::pop_clip();

			drawing::draw_scrollbar(displayable_options_area.size.y, complete_options_area.size.y, m_scroll, { { geometry_area.points.right - 6, geometry_area.points.top + m_rounding }, { 2, geometry_area.size.y - m_rounding * 2 } }, m_rounding, false, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL), m_style->get_color(E_STYLE_COLOR::ACCENT));
		}

		bool c_list_box::process_input(const input::input_capture_t& input, const float scroll)
		{
			if (input::get_is_mouse_in_area(get_geometry_area()))
			{
				if (scroll != 0.f)
				{
					process_scroll(scroll, get_options_area(false, true).size, get_options_area(true, true).size);
					return true;
				}

				if (input.key == VK_LBUTTON && input.get_key_state().is_pressed())
				{
					constexpr int padding = 5;
					math::point_t position = { m_position.x, m_position.y + padding - m_scroll };
					for (std::size_t i = 0; i < m_options.size(); i++)
					{
						if (m_options.at(i).find(m_filter) == std::string::npos)
							continue;

						const math::point_t& option_size = get_option_size(i);
						if (position.y + option_size.y > position.y + m_size.y)
							break; //sorry for this :c

						if (input::get_is_mouse_in_area({ position, option_size }))
						{
							run_input_callback(i);
							return true;
						}

						position.y += option_size.y + padding;
					}
				}
			}

			return false;
		}

		void c_list_box::update()
		{
			//only make it clamp
			if (get_is_visible())
				process_scroll(0.f, get_options_area(false, true).size, get_options_area(true, true).size);
		}

		shapes::rect_t c_list_box::get_options_area(const bool ignore_clip, const bool ignore_scroll)
		{
			auto small_font = m_style->get_font(E_STYLE_FONT::SMALL);

			constexpr int padding = 5;
			math::point_t option_area_size = { m_size.x, padding };
			for (std::size_t i = 0; i < m_options.size(); i++)
			{
				if (option_area_size.y < m_size.y || ignore_clip) //if too many option then stop adding to the box size because it's gonna be clamped anyway.
					option_area_size.y += get_option_size(i).y + padding;
			}

			return { { m_position.x, m_position.y + (ignore_scroll ? 0 : static_cast<int>(m_scroll)) }, option_area_size };
		}
	}
}