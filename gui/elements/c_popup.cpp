#include "c_popup.hpp"
#include "base/c_base_parent.hpp"

namespace rgui
{
	namespace elements
	{
		c_popup::c_popup(const std::string& title, const math::point_t& size)
			: c_base_parent(title, { 0, 0 } /*popups should display in the middle of the parent window so this sis set later*/, size, math::point_t(25, 25)), c_base_animated(math::easing::E_EASING::IN_OUT_SINE, 0, 600)
		{

		}

		void c_popup::draw()
		{
			if (m_is_open)
			{
				static drawing::color_t background_color = drawing::color_t::black();
				background_color.a = 100;

				drawing::draw_rectangle_filled({ m_parent->get_position(), m_parent->get_size() }, background_color);

				const auto& area = get_geometry_area();
				drawing::draw_rectangle_filled(area, m_style->get_color(E_STYLE_COLOR::BACKGROUND));
				drawing::draw_line(area.position, { area.position.x + area.size.x, area.position.y }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL));
				m_style->get_font(E_STYLE_FONT::VERY_SMALL)->draw({ area.position.x + area.size.x / 2, area.position.y + 5 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL),get_title_formatted(), drawing::TEXTFLAG_CENTERED_X);

				drawing::draw_rectangle_filled({ { area.points.right - 6, area.points.top + 1 }, { 6, 6 } }, m_style->get_color(E_STYLE_COLOR::BACKGROUND));
				drawing::draw_circle({ area.points.right - 8, area.points.top + 10 }, 3, m_style->get_color(E_STYLE_COLOR::ACCENT), false);

				drawing::set_clip(area);
				c_base_parent::draw();
				drawing::pop_clip();
			}
		}

		bool c_popup::process_input(const input::input_capture_t& input, const float scroll)
		{
			if (m_is_open)
			{
				if (input.key == VK_ESCAPE && !input::get_key_state(input.key).is_idle())
					m_is_open = false;
				else if (c_base_parent::process_input(input, scroll))
					return true;

				if (input.key == VK_LBUTTON && input.get_key_state().is_pressed())
				{
					if (input::get_is_mouse_in_area(get_geometry_area()))
					{
						if (input::get_is_mouse_in_area({ { m_position.x + m_size.x - 11, m_position.y + 5 }, { 7, 7 } }))
							m_is_open = false;
					}
					else
						m_is_open = false;
				}
			}

			return true;
		}

		void c_popup::update()
		{
			//IT'S IMPORTATNT THAT IT DRAWS OVER ALL THE OTHER ELEMENTS
			if (m_is_open)
				m_parent->set_target_child(shared_from_this());
		}

		void c_popup::set_parent(std::shared_ptr<c_base_parent> parent)
		{
			c_base_element::set_parent(parent);
			const math::point_t& parent_position = m_parent->get_position(), parent_size = m_parent->get_size();
			m_position = { parent_position.x + parent_size.x / 2 - m_size.x / 2, parent_position.y + parent_size.y / 2 - m_size.y / 2 }; //set position to the center of the parent
		}
	}
}