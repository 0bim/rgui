#include "c_base_element.hpp"
#include "c_base_parent.hpp"
#include "c_stylesheet.hpp"

namespace rgui
{
	namespace elements
	{
		c_base_element::c_base_element()
			: m_title(""), m_position({ }), m_size({ }), m_flags(0), m_is_visible(true), m_parent(nullptr), m_port_area({ }), m_rounding(7), m_enable_input(true), m_ignore_scroll(false), m_destruct_post_use(false)
		{
			m_style = &drawing::get_stylesheet()->get_style();
			set_position({ });
		}

		c_base_element::c_base_element(const std::string& title, const math::point_t& position, const math::point_t& size)
			: m_title(title), m_position({ }), m_size(size), m_flags(0), m_is_visible(true), m_parent(nullptr), m_port_area({ }), m_rounding(7), m_enable_input(true), m_ignore_scroll(false), m_destruct_post_use(false)
		{
			m_style = &drawing::get_stylesheet()->get_style();

			assert(m_style != nullptr);

			//because some elements override this
			set_position(position);
		}

		bool c_base_element::get_is_visible() const
		{
			return m_is_visible && (!m_parent || m_parent->get_is_visible());
		}

		bool c_base_element::get_is_target_element()
		{
			//now, it checks if there's nmo parent solely for sliders placed in colorpickers so it always works on them even tho they do not have a set parent
			return !m_parent || m_parent->get_target_child() == shared_from_this();
		}

		shapes::rect_t c_base_element::get_geometry_area_relative() const
		{
			return { get_position() - (m_parent ? m_parent->get_position() : math::point_t(0, 0)), get_size() };
		}

		bool c_base_element::process_input(const char character)
		{
			return false;
		}

		std::string c_base_element::get_title_formatted()
		{
			std::string title = m_title;
			
			std::size_t first = title.find("#");

			while (first != std::string::npos)
			{
				std::size_t last = std::string(title.begin() + first + 1, title.end()).find("#", first);
				title.erase(first, last == std::string::npos ? title.size() - first : last - first + 2);
				first = title.find("#");
			}

			return title;
		}
	}
}