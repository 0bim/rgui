#pragma once
#ifndef C_COLORPICKER_HPP
#define C_COLORPICKER_HPP
#include <array>
#include "base/c_base_parent.hpp"
#include "base/c_base_openable.hpp"
#include "base/c_base_variable_controller.hpp"
#include "base/c_base_value_change_callback.hpp"
#include "c_slider.hpp"
#include "c_text_box.hpp"

namespace rgui
{
	namespace elements
	{
		class c_colorpicker : public c_base_parent, public c_base_openable, public c_base_variable_controller<drawing::color_t>, public c_base_value_change_callback<drawing::color_t>
		{
		public:
			c_colorpicker(const std::string& title, const math::point_t& position, const math::point_t& size = { 14, 14 });

		public:
			void draw() override final;

			bool process_input(const input::input_capture_t& input, const float scroll) override final;

			void update() override final;

			void set_position(const math::point_t& position) override final;

		public:
			inline void set_variable(drawing::color_t* variable) override final
			{
				c_base_variable_controller::set_variable(variable);
				m_opacity_slider->set_variable(&m_variable->a);
				on_color_change();
			}

			inline void set_value(const drawing::color_t value) override final
			{
				c_base_variable_controller::set_value(value);
				on_color_change();
			}

			inline void on_color_change()
			{
				m_color_text.clear();
				std::string str = "";
				str.resize(6);
				drawing::color_t color = m_variable ? *m_variable : drawing::color_t::white();
				snprintf(str.data(), str.size() + 1, "%02x%02x%02x", color.r, color.g, color.b);
				m_color_text = str;

				if (m_variable)
				    run_value_change_callback(color);
			}

			inline void set_draw_title(const bool flag)
			{
				m_draw_title = flag;
			}

			inline bool get_draw_title() const
			{
				return m_draw_title;
			}

		public:
			static IDirect3DTexture9* get_picker_box_texture(const int hue);

			//static void get_picker_box_textures();

			//inline static std::vector<std::future<IDirect3DTexture9*>> m_texture_setup_futures = { };

		private:
			static IDirect3DTexture9* get_hue_texture();

		private:
			inline math::point_t get_background_box_postion()
			{
				return { m_position.x + m_size.x + 5, m_position.y };
			}

		private:
			std::shared_ptr<c_slider<std::uint8_t>> m_opacity_slider;
			std::shared_ptr<c_text_box> m_text_box;
			std::string m_color_text;

			bool m_draw_title;
		};
	}
}

#endif