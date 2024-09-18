#include "c_groupbox.hpp"
#include "base/c_base_form.hpp"

namespace rgui
{
	namespace elements
	{
		c_groupbox::c_groupbox(const std::string& title, const math::point_t& position, const math::point_t& size, const char icon) 
			: c_base_parent(title, position, size, math::point_t(15, 50)), c_base_icon(icon), m_auto_resize(false)
		{

		}

		c_groupbox::c_groupbox(const std::string& title, const math::point_t& position, const char icon)
			: c_base_parent(title, position, math::point_t(285, 50), math::point_t(15, 50)), c_base_icon(icon), m_auto_resize(true)
		{

		}

		void c_groupbox::draw()
		{
			const auto& area = get_geometry_area();

			drawing::draw_rectangle_filled(area, m_style->get_color(E_STYLE_COLOR::BACKGROUND_LIGHT), 10);

			std::string icon_str = { m_icon };
			const auto& icon_size = m_style->get_font(E_STYLE_FONT::LARGE_ICON)->get_text_size(icon_str), title_size = m_style->get_font(E_STYLE_FONT::LARGE)->get_text_size(get_title_formatted());
			m_style->get_font(E_STYLE_FONT::LARGE_ICON)->draw({ area.position.x + 15, area.position.y + 10 }, m_style->get_color(E_STYLE_COLOR::ACCENT), icon_str);
			const int icon_spacing = m_icon == -1 ? 0 : 5;
			m_style->get_font(E_STYLE_FONT::LARGE)->draw({ area.position.x + icon_spacing + icon_size.x + 15.f, area.position.y + icon_size.y * 0.5f + 10 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_HIGHLIGHTED), get_title_formatted(), drawing::TEXTFLAG_CENTERED_Y);

			shapes::rect_t clip_area = { { area.position.x, area.position.y + (m_spacing.y * 0.75f) }, { area.size.x, area.size.y - (m_spacing.y * 0.75f) } };

			drawing::set_clip(clip_area);

			c_base_parent::draw();

			if (get_auto_resize())
				force_auto_resize();

			//if (get_is_scrollable() && get_max_horizontal_scroll() > get_size().y)
				//drawing::draw_rectangle_filled({ { area.points.left, area.points.bottom - area.size.y * 0.25f }, { area.size.x, area.size.y * 0.25f } }, { 0, 0, 0, 50 }, m_rounding);

			//if (scrollable)
			drawing::pop_clip();
		}

		bool c_groupbox::process_input(const input::input_capture_t& input, const float scroll)
		{
			const auto& area = get_geometry_area();

			drawing::set_clip({ { area.position.x, area.position.y + (m_spacing.y * 0.75f) }, { area.size.x, area.size.y - (m_spacing.y * 0.75f) } });

			bool flag = c_base_parent::process_input(input, scroll) || (input::get_is_mouse_in_area(get_geometry_area()) && scroll == 0.f && !c_base_draggable::get_currently_dragged());

			drawing::pop_clip();

			return flag;
		}

		void c_groupbox::update()
		{
			c_base_parent::update();
		}
	}
}