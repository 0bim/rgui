#include "c_base_parent.hpp"
#include "../elements.hpp"

namespace rgui
{
	namespace elements
	{
		int c_base_parent::get_previous_child_height(const bool sameline) const
		{
			if (m_children.empty())
				return m_spacing.y;

			const auto& area = m_children.back()->get_geometry_area();
			return (sameline ? area.points.top : area.points.bottom) - m_position.y /*take away the parent's position*/;
		}

		math::point_t c_base_parent::calculate_auto_position(const int space_between, const bool sameline) const
		{
			return { m_spacing.x, get_previous_child_height(sameline) + (m_children.empty() ? space_between * 0.4f : space_between) + m_extra_space };
		}

		std::shared_ptr<c_button> c_base_parent::button(const std::string& title, const math::point_t& size, const int space_between)
		{
			auto button = std::make_shared<c_button>(title, calculate_auto_position(space_between), size);
			push_child(button);
			return button;
		}

		std::shared_ptr<c_checkbox> c_base_parent::checkbox(const std::string& title, bool* variable, const int space_between)
		{
			auto checkbox = std::make_shared<c_checkbox>(title, calculate_auto_position(space_between + 2));
			checkbox->set_variable(variable);
			push_child(checkbox);
			return checkbox;
		}

		std::shared_ptr<c_switchbox> c_base_parent::switchbox(const std::string& title, bool* variable, const int space_between)
		{
			auto switchbox = std::make_shared<c_switchbox>(title, calculate_auto_position(space_between + 2));
			switchbox->set_variable(variable);
			push_child(switchbox);
			return switchbox;
		}

		std::shared_ptr<c_colorpicker> c_base_parent::colorpicker(const std::string& title, drawing::color_t* variable, const bool draw_title, const bool same_line, const int space_between, const int margin)
		{
			auto position = calculate_auto_position(space_between, same_line);
			if (!m_children.empty() && m_children.back())
				position.x = m_size.x - m_spacing.x - (same_line ? m_children.back()->get_size().x : 0) - margin;

			auto colorpicker = std::make_shared<c_colorpicker>(title, position);
			colorpicker->set_variable(variable);
			colorpicker->set_draw_title(draw_title);
			push_child(colorpicker);
			return colorpicker;
		}

		std::shared_ptr<c_dropdown> c_base_parent::dropdown(const std::string& title, const int space_between)
		{
			auto dropdown = std::make_shared<c_dropdown>(title, calculate_auto_position(space_between));
			push_child(dropdown);
			return dropdown;
		}

		std::shared_ptr<c_dropdown_elements> c_base_parent::dropdown_elements(const std::string& title, const int child_panel_height, const int space_between)
		{
			auto dropdown_elements = std::make_shared<c_dropdown_elements>(title, child_panel_height, calculate_auto_position(space_between));
			push_child(dropdown_elements);
			return dropdown_elements;
		}

		std::shared_ptr<c_list_box> c_base_parent::list_box(const std::string& title, const int height, const int space_between)
		{
			auto list_box = std::make_shared<c_list_box>(title, calculate_auto_position(space_between));
			list_box->set_size({ list_box->get_size().x, height });
			push_child(list_box);
			return list_box;
		}

		std::shared_ptr<c_keybinder> c_base_parent::keybinder(const std::string& title, input::custom_keybind_t* variable, const int space_between, const int margin)
		{
			auto position = calculate_auto_position(space_between, true);
			if (!m_children.empty() && m_children.back())
				position.x = m_size.x - m_spacing.x - m_children.back()->get_size().x - margin;

			position.y += 2;

			auto keybinder = std::make_shared<c_keybinder>(title, position);
			keybinder->set_variable(variable);
			push_child(keybinder);
			return keybinder;
		}

		template<typename T>
		std::shared_ptr<c_slider<T>> c_base_parent::slider(const std::string& title, T* variable, const int space_between)
		{
			auto slider = std::make_shared<c_slider<T>>(title, calculate_auto_position(space_between));
			slider->set_variable(variable);
			push_child(slider);
			return slider;
		}

		std::shared_ptr<c_text_box> c_base_parent::text_box(const std::string& title, std::string* variable, const int space_between)
		{
			auto text_box = std::make_shared<c_text_box>(title, calculate_auto_position(space_between));
			text_box->set_variable(variable);
			push_child(text_box);
			return text_box;
		}

		template<input_number_t T>
		std::shared_ptr<c_input_box<T>> c_base_parent::input_box(const std::string& title, T* variable , const int space_between)
		{
			auto input_box = std::make_shared<c_input_box<T>>(title, calculate_auto_position(space_between));
			push_child(input_box);
			return input_box;
		}

		std::shared_ptr<c_text> c_base_parent::text(const std::string& title, const math::point_t& position, drawing::font_t* font, const E_STYLE_COLOR color_index, const std::uint8_t text_flags, const int space_between)
		{
			auto text = std::make_shared<c_text>(title, position, font, color_index, text_flags);
			push_child(text);
			return text;
		}

		std::shared_ptr<c_groupbox> c_base_parent::groupbox(const std::string& title, const math::point_t& position, const char icon)
		{
			auto groupbox = std::make_shared<c_groupbox>(title, position, icon);
			push_child(groupbox);
			return groupbox;
		}

		template std::shared_ptr<c_slider<float>> c_base_parent::slider<float>(const std::string& title, float* variable, const int space_between);
		template std::shared_ptr<c_slider<int>> c_base_parent::slider<int>(const std::string& title, int* variable, const int space_between);
		template std::shared_ptr<c_slider<std::uint8_t>> c_base_parent::slider<std::uint8_t>(const std::string& title, std::uint8_t* variable, const int space_between);

		template std::shared_ptr<c_input_box<float>> c_base_parent::input_box(const std::string& title, float* variable, const int space_between);
		template std::shared_ptr<c_input_box<int>> c_base_parent::input_box(const std::string& title, int* variable, const int space_between);
		template std::shared_ptr<c_input_box<std::uint8_t>> c_base_parent::input_box(const std::string& title, std::uint8_t* variable, const int space_between);
	}
}