#include "c_base_form.hpp"

namespace rgui
{
	namespace elements
	{
		c_base_form::c_base_form(const std::string& title, const math::point_t& position, const math::point_t& size)
			: c_base_parent(title, position, size, { 0, 20 }), c_base_draggable(), m_is_draggable(true), m_open_animation({ }), m_scroll_animation({ }), m_reverse_scroll_animation(false), m_background_texture(nullptr), m_is_outlined(false), m_fill_background(true)
		{
			m_rounding = 10;
		}

		void c_base_form::draw()
		{
			const auto& area = get_geometry_area();

			//clip area
			drawing::set_clip({ area.position - math::point_t(2, 30), area.size + math::point_t(4, 32) }, true);

			if (!get_title_formatted().empty())
			{
				const auto& title_text_size = m_style->get_font(E_STYLE_FONT::SMALL)->get_text_size(get_title_formatted());
				shapes::rect_t top_bar_area = { { area.position.x - (m_is_outlined ? 2 : 0), area.position.y - title_text_size.y * 2 }, { area.size.x + (m_is_outlined ? 4 : 0), title_text_size.y * 2 + (m_fill_background ? m_rounding * 1.5f : 0.f) } };
				drawing::draw_rectangle_filled(top_bar_area, m_style->get_color(E_STYLE_COLOR::BACKGROUND_DARK), m_rounding);

				m_style->get_font(E_STYLE_FONT::SMALL)->draw(top_bar_area.position + math::point_t(top_bar_area.size.x * 0.5f, (top_bar_area.size.y - (m_fill_background ? m_rounding * 1.5f : 0.f)) * 0.5f), m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL), get_title_formatted(), drawing::TEXTFLAG_CENTERED_X | drawing::TEXTFLAG_CENTERED_Y);
			}

			if (m_background_texture)
				drawing::draw_image_raw(area, m_background_texture, m_rounding);
			else
				drawing::draw_rectangle_filled(area, m_style->get_color(E_STYLE_COLOR::BACKGROUND), m_rounding);

			if (m_is_outlined)
			{
				drawing::draw_rectangle({ area.position - 1, area.size + 2 }, m_style->get_color(E_STYLE_COLOR::BACKGROUND_DARK), m_rounding);
				drawing::draw_rectangle({ area.position - 2, area.size + 4 }, m_style->get_color(E_STYLE_COLOR::BACKGROUND_DARK), m_rounding);
			}

			if (!m_scroll_animation.get_has_ended(500))
			{
				const int shaft_width = 5;
				const float blend = m_scroll_animation.ease(math::easing::E_EASING::IN_BOUNCE, 500, m_reverse_scroll_animation);
				if (blend > 0.3f)
				{
					shapes::rect_t shaft_area = { { area.points.right - (shaft_width + 1) * blend, area.points.top + m_rounding }, { shaft_width * blend, area.size.y - m_rounding * 2 } };
					draw_scrollbar(shaft_area, m_rounding, false);
				}
			}
			else if (!m_reverse_scroll_animation)
			{
				m_reverse_scroll_animation = true;
				m_scroll_animation.begin();
			}

			drawing::pop_clip();

			drawing::set_clip(area, true);
			//drawing::draw_rectangle_filled({ m_position, { 45, m_size.y } }, m_style->get_color(E_STYLE_COLOR::BACKGROUND_OVERLAY), 10);
			//drawing::draw_rectangle_filled({ { m_position.x + 45, m_position.y }, { m_size.x - 45, 45 } }, m_style->get_color(E_STYLE_COLOR::BACKGROUND_GROUP));
			//drawing::draw_line({ m_position.x, m_position.y + 45 }, { m_position.x + m_size.x, m_position.y + 45 }, m_style->get_color(E_STYLE_COLOR::LIGHT_GREY));
			//drawing::draw_line({ m_position.x + 45, m_position.y }, { m_position.x + 45, m_position.y + m_size.y }, m_style->get_color(E_STYLE_COLOR::LIGHT_GREY));

			c_base_parent::draw();

			drawing::pop_clip();
		}

		bool c_base_form::process_input(const input::input_capture_t& input, const float scroll)
		{
			const int backup_scroll = m_scroll;
			if (m_is_being_dragged || !c_base_parent::process_input(input, scroll))
			{
				if (input.key == VK_LBUTTON)
				{
					if (input.get_key_state().is_idle())
						set_is_being_dragged(false);
					else if (m_is_being_dragged)
					{
						if (m_is_draggable)
						{
							const math::point_t& delta = input::get_mouse_pos() - m_drag_begin_point;
							on_move(delta);
							m_drag_begin_point = input::get_mouse_pos();
							return true;
						}
					}
					else
					{
						const auto& area = get_geometry_area();
						const auto& title_text_size = m_style->get_font(E_STYLE_FONT::SMALL)->get_text_size(get_title_formatted());
						shapes::rect_t top_bar_area = { { area.position.x - 2, area.position.y - title_text_size.y * 2 }, { area.size.x + 4, title_text_size.y * 2 + m_rounding * 1.5f } };
						const shapes::rect_t drag_area = { top_bar_area.position, math::point_t(top_bar_area.size.x, area.size.y + top_bar_area.size.y) };
						if (m_is_draggable && input::get_is_mouse_in_area(drag_area))
						{
							if (input.get_key_state().is_held())
								if (set_is_being_dragged(true))
									return true;
						}
						else if (input.get_key_state().is_pressed())
							return false;
					}
				}

				return false;
			}

			if (backup_scroll != m_scroll)
				m_scroll_animation.begin();

			set_is_being_dragged(false);
			return true;
		}

		bool c_base_form::process_input(const char character)
		{
			return c_base_parent::process_input(character);
		}

		void c_base_form::update()
		{
			c_base_parent::update();
		}
	}
}