#include "c_colorpicker.hpp"
#include "base/c_base_parent.hpp"

namespace rgui
{
	namespace elements
	{
		inline drawing::color_t m_clipboard_color = { };
		inline std::array<IDirect3DTexture9*, 361> m_picker_box_textures = { };

		static math::point_t m_picker_box_size = { 160, 160 }, m_hue_box_size = { 10, 160 }, m_background_box_size = { 200, 250 };

		c_colorpicker::c_colorpicker(const std::string& title, const math::point_t& position, const math::point_t& size)
			: c_base_parent(title, position, size), c_base_variable_controller(), c_base_value_change_callback(), m_color_text(""), m_draw_title(false)
		{
			auto ptr = std::shared_ptr<c_colorpicker>(this, [](c_colorpicker*) {});

			m_spacing = { 0, 0 };

			const auto size_backup = m_size;

			set_size(m_picker_box_size);

			const math::point_t& background_box_position = get_background_box_postion();
			m_opacity_slider = slider<std::uint8_t>("");
			m_opacity_slider->set_size(math::point_t(160, 5));
			m_opacity_slider->set_marker_radius(5);
			m_opacity_slider->set_limits(std::numeric_limits<std::uint8_t>::min(), std::numeric_limits<std::uint8_t>::max());
			m_opacity_slider->set_input_callback([=]()
				{
					on_color_change();
				});

			push_child(m_opacity_slider);

			m_text_box = text_box("");
			m_text_box->set_variable(&m_color_text);
			m_text_box->set_input_callback([this]()
				{
					if (!this->m_text_box->get_is_open())
					{
						if (this->m_color_text.size() == 6)
						{
							int r = 0, g = 0, b = 0, a = this->m_variable->a;
							std::sscanf(this->m_color_text.data(), "%02x%02x%02x", &r, &g, &b);
							this->set_value({ r, g, b, a });
						}
						else
							this->on_color_change();
					}
				});
			push_child(m_text_box);

			set_size(size_backup);

			on_color_change();
		}

		void c_colorpicker::draw()
		{
			if (m_is_open)
			{
				const math::point_t background_box_position = get_background_box_postion();
				drawing::set_clip({ background_box_position, m_background_box_size }, true);

				//draw background (add 20 as padding 10 top/bottom and 10 on left/right
				drawing::color_t background = m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND);
				background.a = 220.f;
				drawing::draw_rectangle_filled({ background_box_position, m_background_box_size }, background);

				const drawing::color_hsv_t hsv = m_variable->to_hsv();
				//picker box
				const shapes::rect_t picker_box_area = { { background_box_position.x + 10, background_box_position.y + 10 }, m_picker_box_size };
				drawing::draw_image_raw(picker_box_area, get_picker_box_texture(static_cast<int>(hsv.h)));
				drawing::draw_rectangle_filled({ { picker_box_area.points.left + m_picker_box_size.x * (hsv.v / 100) - 2, picker_box_area.points.bottom - m_picker_box_size.y * (hsv.s / 100) - 2 }, { 4, 4 } }, m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND));

				//hue bar
				const shapes::rect_t hue_bar_area = { { background_box_position.x + m_background_box_size.x - m_hue_box_size.x - 10, background_box_position.y + 10 }, m_hue_box_size };
				drawing::draw_image_raw(hue_bar_area, get_hue_texture());
				drawing::draw_rectangle_filled({ { hue_bar_area.points.left, hue_bar_area.points.top + static_cast<int>(m_hue_box_size.y * (hsv.h / 360)) }, { hue_bar_area.size.x, 2 } }, drawing::color_t::black());

				c_base_parent::draw();

				drawing::pop_clip();

				/*drawing::color_hsv_t hsv = { 0.f, 100.f, 100.f };
				const float box_height = m_hue_box_size.y / 360.f;
				for (int i = 0; i <= m_hue_box_size.y; i++)
				{
					drawing::draw_rectangle_filled({ background_box_position.x + m_background_box_size.x - m_hue_box_size.x - 10.f, background_box_position.y + box_height * i + 10 }, { static_cast<float>(m_hue_box_size.x), (box_height) }, hsv.to_rgb());
					hsv.h += 360.f / m_hue_box_size.y;
				}*/
				//drawing::draw_image(get_hue_texture())
			}

			const auto& area = get_geometry_area();

			math::point_t circle_position = area.position + area.size * 0.5f - math::point_t(0, 2);
			m_style->get_font(E_STYLE_FONT::SMALL_ICON)->draw(circle_position, m_style->get_color(get_is_open() ? E_STYLE_COLOR::ELEMENT_HIGHLIGHTED : E_STYLE_COLOR::ELEMENT_GENERAL), std::string("n"), drawing::TEXTFLAG_CENTERED_X | drawing::TEXTFLAG_CENTERED_Y);
			/*circle_position.y -= 1;
			auto circle_color = get_value();
			circle_color.a = std::min(circle_color.a, m_style->get_color(E_STYLE_COLOR::ELEMENT_BACKGROUND).a);
			drawing::draw_circle(circle_position, area.size.y * 0.5f, circle_color, true);*/

			//drawing::draw_rectangle_filled(get_geometry_area(), *m_variable);

			if (m_draw_title)
			    m_style->get_font(E_STYLE_FONT::SMALL)->draw({ area.points.right + 5, m_position.y + m_size.y / 2 }, m_style->get_color(E_STYLE_COLOR::ELEMENT_GENERAL), get_title_formatted(), drawing::TEXTFLAG_CENTERED_Y);
		}

		bool c_colorpicker::process_input(const input::input_capture_t& input, const float scroll)
		{
			if (input.key == VK_LBUTTON) //the only thing that's needed for now
			{
				//if (!input.get_key_state().is_held())
					//m_opacity_slider->set_is_being_dragged(false);

				const math::point_t background_box_position = get_background_box_postion();

				if (input::get_is_mouse_in_area(get_geometry_area()))
				{
					if (input.get_key_state().is_pressed())
					{
						m_is_open = true;
						m_opacity_slider->set_position(math::point_t(background_box_position.x + 10, background_box_position.y + m_picker_box_size.y + 35));
						m_text_box->set_position(math::point_t(background_box_position.x + 10, background_box_position.y + m_picker_box_size.y + m_opacity_slider->get_size().y + 40));
						return true;
					}
				}
				else if (m_is_open)
				{
					if (input::get_is_mouse_in_area({ background_box_position, m_background_box_size })) //is the cursor in the background area to begin with before checking everything else.
					{
						if (!input.get_key_state().is_idle()) //if isn't idle (pressed/held)
						{
							const math::point_t cursor_position = input::get_mouse_pos();

							//picker box
							if (const shapes::rect_t area = { { background_box_position.x + 10, background_box_position.y + 10 }, m_picker_box_size }; input::get_is_mouse_in_area(area))
							{
								drawing::color_hsv_t color = { m_variable->to_hsv().h, 100.f * std::abs(cursor_position.y - area.points.bottom) / m_picker_box_size.y, 100.f * std::abs(cursor_position.x - area.points.left) / m_picker_box_size.x };
								const std::uint8_t alpha = m_variable->a;
								set_value(color.to_rgb());
								m_variable->a = alpha;
							}
							//hue bar
							else if (const shapes::rect_t area = { { background_box_position.x + m_background_box_size.x - m_hue_box_size.x - 10, background_box_position.y + 10 }, { m_hue_box_size.x, m_hue_box_size.y } }; input::get_is_mouse_in_area(area))
							{
								const float hue = 360.f * std::abs(cursor_position.y - area.points.top) / m_hue_box_size.y;
								const std::uint8_t alpha = m_variable->a;
								const drawing::color_hsv_t& backup_hsv = m_variable->to_hsv();
								drawing::color_hsv_t color = { hue, backup_hsv.s, backup_hsv.v };
								set_value(color.to_rgb());
								m_variable->a = alpha;
							}
							else //if not chaning color then they might be chaning opacity
							{
								c_base_parent::process_input(input, scroll);
							}
						}
						else //if is idle also run input
							c_base_parent::process_input(input, scroll);
					}
					else if (input.get_key_state().is_pressed()) //don't close if holding
					{
						m_is_open = false;
						m_text_box->set_is_open(false);
					}

					return true;
				}
			}
			else
			    c_base_parent::process_input(input, scroll);

			return false;
		}

		void c_colorpicker::update()
		{

		}

		void c_colorpicker::set_position(const math::point_t& position)
		{
			c_base_element::set_position(position);
			const auto& background_box_position = get_background_box_postion();
			m_opacity_slider->set_position(math::point_t(background_box_position.x + 10, background_box_position.y + m_picker_box_size.y + 35));
			m_text_box->set_position(math::point_t(background_box_position.x + 10, background_box_position.y + m_picker_box_size.y + m_opacity_slider->get_size().y + 40));
		}

		IDirect3DTexture9* c_colorpicker::get_hue_texture()
		{
			static IDirect3DTexture9* texture = nullptr;
			static std::unique_ptr<drawing::color_t[]> data = nullptr;

			if (texture)
				return texture;

			if (!data)
			{
				data = std::make_unique<drawing::color_t[]>(m_hue_box_size.x * m_hue_box_size.y);

				for (std::size_t i = 0; i < m_hue_box_size.x; i++)
				{
					float hue = 0.f;
					for (std::size_t j = 0; j < m_hue_box_size.y; j++)
					{
						hue += 360.f / m_hue_box_size.y;
						*static_cast<drawing::color_t*>(data.get() + i + j * m_hue_box_size.x) = drawing::color_hsv_t(hue, 100.f, 100.f).to_rgb();
					}
				}
			}

			texture = static_cast<IDirect3DTexture9*>(rgui::drawing::create_texture_rgba(data.get(), m_hue_box_size));

			return texture;
		}

		IDirect3DTexture9* c_colorpicker::get_picker_box_texture(const int hue)
		{
			if (hue < m_picker_box_textures.size() && m_picker_box_textures[hue])
				return m_picker_box_textures[hue];

			std::unique_ptr<drawing::color_t[]> data = std::make_unique<drawing::color_t[]>(m_picker_box_size.x * m_picker_box_size.y);

			float saturation = 0.f, visibility = 0.f;
			for (std::size_t i = 0; i < m_picker_box_size.x; i++)
			{
				visibility += 100.f / m_picker_box_size.x;
				saturation = 100.f;
				for (std::size_t j = 0; j < m_picker_box_size.y; j++)
				{
					saturation -= 100.f / m_picker_box_size.y;
					*static_cast<drawing::color_t*>(data.get() + i + j * m_picker_box_size.x) = drawing::color_hsv_t(hue, saturation, visibility).to_rgb();
				}
			}

			auto texture = static_cast<IDirect3DTexture9*>(rgui::drawing::create_texture_rgba(data.get(), m_picker_box_size));
			m_picker_box_textures[hue] = texture;

			return texture;
		}

		/*void c_colorpicker::get_picker_box_textures()
		{
			m_texture_setup_futures.reserve(360);
			for (std::size_t i = 0; i < 360; i++)
				m_texture_setup_futures.emplace_back(std::async(std::launch::async, get_picker_box_texture, i));
		}*/
	}
}