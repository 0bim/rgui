#pragma once
#ifndef RENDER_HPP
#define RENDER_HPP

#ifdef RENDERER_OLD
#include <string>
#include <vector>
#include "font_t.hpp"
#include "c_texture.hpp"
#include "../color/color.hpp"
#include "../../helpers/input/input.hpp"
#include "../../helpers/math/math.hpp"
#include "animated_gif.hpp"

namespace rgui
{
	namespace elements
	{
		class c_base_element;
		class c_base_form;
		class c_stylesheet;
	}

	namespace drawing
	{
		//inline c_directx9_renderer directx9_renderer;

		enum class E_ANIMATION
		{
			NONE = (1 << 0),
			ADJUST_SIZE = (1 << 1),
			MOVE = (1 << 2),
			FADE_IN = (1 << 3)
		};

		enum class E_TEXTBOX_FLAGS
		{
			SECRET = (1 << 0),
			LIMITED = (1 << 1),
			NORMAL = (1 << 2)
		};

		/*enum class E_MENU_COLORS
		{
			WINDOW_BG_COLOR = 0,
			WINDOW_OUTLINE_COLOR,
			WINDOW_NAVBAR_BG_COLOR,
			WINDOW_NAVBAR_TEXT_COLOR,
			WINDOW_NAVBAR_OUTLINE_COLOR,

			TAB_BG_COLOR,
			TAB_HOVERED_COLOR,
			TAB_SELECTED_COLOR,
			TAB_OUTLINE_COLOR,
			TAB_TEXT_COLOR,

			CHECKBOX_BG_COLOR,
			CHECKBOX_OUTLINE_COLOR,
			CHECKBOX_TEXT_COLOR,
			CHECKBOX_CHECK_MARK,

			SLIDER_BG_COLOR,
			SLIDER_FILL_COLOR,
			SLIDER_FILL_BOX_COLOR,
			SLIDER_TEXT_COLOR,
			SLIDER_OUTLINE_COLOR,

			BUTTON_BG_COLOR,
			BUTTON_BG_HOVERED_COLOR,
			BUTTON_OUTLINE_COLOR,
			BUTTON_TEXT_COLOR,

			GROUPBOX_TOP_BAR_COLOR,
			GROUPBOX_BG_COLOR,
			GROUPBOX_TEXT_COLOR,

			TEXTBOX_TEXT_COLOR,
			TEXTBOX_TEXT_UNDERLINE_COLOR,
			TEXTBOX_BG_COLOR,
			TEXTBOX_OUTLINE_COLOR,
			TEXTBOX_SELECTED_OUTLINE_COLOR,

			COMBOBOX_BG_COLOR,
			COMBOBOX_OUTLINE_COLOR,
			COMBOBOX_TEXT_COLOR,
			COMBOBOX_SELECTED_TEXT_COLOR,

			MULTISELECT_BG_COLOR,
			MULTISELECT_OUTLINE_COLOR,
			MULTISELECT_TEXT_COLOR,
			MULTISELECT_SELECTED_TEXT_COLOR,

			COLOR_MAX
		};

		namespace function_templates
		{
			using draw_rectangle_t = std::add_pointer_t<void(const shapes::rect_t& rect, const color_t& color)>;
			using draw_gradient_rectangle_t = std::add_pointer_t<void(const shapes::rect_t& rect, const color_t& color_begin, const color_t& color_gradient, const bool horizontal)>;
			using create_font_t = std::add_pointer_t<std::size_t()>;
			using delete_font_t = std::add_pointer_t<void(const std::size_t font_id)>;
			using is_font_id_valid_t = std::add_pointer_t<bool(const std::size_t font_id)>;
			//using setup_font_t = std::add_pointer_t<void(const std::size_t font_id, const std::string_view type_name, const std::size_t tall, const std::size_t weight, const std::uint8_t setup_flags)>;
			using draw_text_t = std::add_pointer_t<void(math::point_t position, const std::string& text, const size_t font_id, const color_t& color, const std::uint8_t flags)>;
			using get_text_size_t = std::add_pointer_t<math::point_t(const std::string& text, const size_t font_id)>;
			using set_clip_t = std::add_pointer_t<void(const shapes::rect_t& area)>;
			using pop_clip_t = std::add_pointer_t<void()>;
			using set_clip_flag_t = std::add_pointer_t<void(const bool flag)>;
			using get_tick_count_t = std::add_pointer_t<int()>;
			using create_texture_t = std::add_pointer_t<int(const bool flag)>;
			using set_texture_t = std::add_pointer_t<void(const int id, color_t* texture, const math::point_t& dimensions)>;
			using get_screen_size_t = std::add_pointer_t<math::point_t()>;
		}*/

		class c_render
		{
		public:
			struct batch_t
			{
				std::size_t m_vertex_count;
				D3DPRIMITIVETYPE m_primitive_type;
				shapes::rect_t m_clip;
				IDirect3DTexture9* m_texture;
				bool m_is_text;
			};

			struct draw_list_t
			{
				inline void reserve(const std::size_t vertices, const std::size_t batches)
				{
					m_vertices.reserve(vertices);
					m_batches.reserve(batches);
				}

				inline void push(const std::vector<math::vertex_t>& vertices, D3DPRIMITIVETYPE primitive_type, const shapes::rect_t& clip, IDirect3DTexture9* texture = nullptr, const bool text = false)
				{
					m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());

					if (!m_batches.empty())
					{
						auto& last_batch = m_batches.back();
						if (last_batch.m_primitive_type != D3DPT_LINESTRIP && last_batch.m_primitive_type != D3DPT_TRIANGLEFAN && last_batch.m_primitive_type == primitive_type && !last_batch.m_texture && !texture && last_batch.m_clip == clip)
						{
							last_batch.m_vertex_count += vertices.size();
							return;
						}
					}

					m_batches.emplace_back(vertices.size(), primitive_type, clip, texture, text);
				}

				inline void push(const math::vertex_t& vertex, D3DPRIMITIVETYPE primitive_type, const shapes::rect_t& clip, IDirect3DTexture9* texture = nullptr, const bool text = false)
				{
					m_vertices.push_back(vertex);

					if (!m_batches.empty())
					{
						auto& last_batch = m_batches.back();
						if (last_batch.m_primitive_type == primitive_type && !last_batch.m_texture && !texture && last_batch.m_clip == clip)
						{
							last_batch.m_vertex_count += 1;
							return;
						}
					}

					m_batches.emplace_back(1, primitive_type, clip, texture, text);
				}

				inline void clear()
				{
					m_vertices.clear();
					m_batches.clear();
				}

				std::vector<math::vertex_t> m_vertices;
				std::vector<batch_t> m_batches;
			};

		public:
			c_render(IDirect3DDevice9* device = nullptr)
				: m_device(device), m_state_block(nullptr), m_vertex_buffer(nullptr), m_draw_list({ }), m_clip_records({}), m_custom_fonts({}), m_forms({ }), m_stylesheet(nullptr), m_previous_element_height(0), m_frame_rate(1), m_frame_count(1), m_precached_texture({ })
			{
				set_device(device);
				m_fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
			};

			~c_render()
			{
				if (m_state_block)
					m_state_block->Release();

				if (m_vertex_buffer)
					m_vertex_buffer->Release();
			};

			//direct x functions

			void set_device(IDirect3DDevice9* device);

			IDirect3DDevice9* get_device() const;

			void begin();

			void end();

			void draw();

			void set_screen_resolution(const math::point_t& resolution);

			math::point_t get_screen_resolution();

			void set_clip(const shapes::rect_t& rect);

			void pop_clip();

			shapes::rect_t get_clip();

			shapes::rect_t get_clip_active();

			IDirect3DTexture9* create_texture_rgba(const color_t* data, const math::point_t& size);

			IDirect3DTexture9* create_texture_file_memory(const std::uint8_t* data, const std::size_t size);

			IDirect3DTexture9* create_texture_file(const std::string& path);

			IDirect3DTexture9* create_texture_file_svg_memory(std::uint8_t* data, math::point_t* size = nullptr);

			IDirect3DTexture9* create_texture_file_svg(const std::string& path, math::point_t* size = nullptr);

			void precache_texture(const std::uint32_t hash, const c_texture& texture);

			void precache_texture(const std::uint32_t hash, IDirect3DTexture9* texture_d3d, const math::point_t& size);

			bool get_is_texture_precached(const std::uint32_t hash);

			c_texture get_texture_precached(const std::uint32_t hash);

			void register_font(font_t* font);

			void unregister_font(font_t* font);

			custom_font_id_t register_custom_font(const font_t& font);

			font_t& get_custom_font(const custom_font_id_t id);

			std::vector<font_t>& get_custom_fonts();

			bool is_custom_font_id_valid(const custom_font_id_t id);

			void on_reset();

			void draw_pixel(const math::point_t& position, const color_t& color);

			void wu_pixel(const math::point_t& position, const color_t& color, const float c);

			void draw_line(const math::point_t& pos1, const math::point_t& pos2, const color_t& color);

			void draw_rectangle_filled(const shapes::rect_t& rect, const color_t& color, const bool aa = true);

			void draw_rectangle_filled(const shapes::rect_t& rect, const color_t& color, const color_t& color_gradient, const bool horizontal);

			void draw_image(const shapes::rect_t& rect, IDirect3DTexture9* texture, const color_t& color);

			void draw_image(const math::point_t& position, const c_texture& texture, const color_t& color);

			void draw_image(const shapes::rect_t& rect, IDirect3DTexture9* texture);

			void draw_image(const math::point_t& position, const c_texture& texture);

			void draw_rectangle(const shapes::rect_t& rect, const color_t& color);

			void draw_circle(const math::point_t& position, const int radius, const c_texture& texture, const color_t& color = color_t::white());

			void draw_circle(const math::point_t& position, const int radius, const color_t& color, const bool filled);

			void draw_circle_annulus(const math::point_t& position, const int radius_in, const int radius_out, const float blend, const color_t& color);

			void draw_element(std::shared_ptr<elements::c_base_element> element);

			void draw_form(std::shared_ptr<elements::c_base_form> form);

		public:
			inline void push_form(std::shared_ptr<elements::c_base_form> form)
			{
				m_forms.push_back(form);
			}

			inline std::vector<std::shared_ptr<elements::c_base_form>> get_forms()
			{
				return m_forms;
			}

			inline void set_stylesheet(std::shared_ptr<elements::c_stylesheet> stylesheet)
			{
				m_stylesheet = stylesheet;
			}

			inline std::shared_ptr<elements::c_stylesheet> get_stylesheet()
			{
				return m_stylesheet;
			}

			inline int get_fvf() const
			{
				return m_fvf;
			}

			inline draw_list_t& get_draw_list()
			{
				return m_draw_list;
			}

			inline void set_previous_element_height(const int height)
			{
				m_previous_element_height = height;
			}

			inline int get_previous_element_height() const
			{
				return m_previous_element_height;
			}

			inline int get_frame_rate() const
			{
				return std::max(1, m_frame_rate);
			}

			inline custom_font_id_t get_max_font_id() const
			{
				return m_custom_fonts.size();
			}

		private:
			IDirect3DDevice9* m_device;
			IDirect3DStateBlock9* m_state_block;
			IDirect3DVertexBuffer9* m_vertex_buffer;
			draw_list_t m_draw_list;

			math::point_t m_screen_resolution;

			std::vector<shapes::rect_t> m_clip_records;

			std::vector<font_t> m_custom_fonts;
			std::vector<font_t*> m_managed_fonts;
			std::vector<std::shared_ptr<elements::c_base_form>> m_forms;
			std::shared_ptr<elements::c_stylesheet> m_stylesheet;

			std::chrono::system_clock::time_point m_frame_timestamp;
			int m_frame_count;
			int m_frame_rate;

			int m_fvf;

			int m_previous_element_height;

			std::map<std::uint32_t, c_texture> m_precached_texture;
		};

		inline std::shared_ptr<c_render> renderer = std::make_shared<c_render>();
		//inline funcs_t funcs;
	}
}

#else
#include <string>
#include <vector>
#include "font_t.hpp"
#include "c_texture.hpp"
#include "../color/color.hpp"
#include "../../helpers/input/input.hpp"
#include "../../helpers/math/math.hpp"
#include "animated_gif.hpp"

namespace rgui
{
	namespace elements
	{
		class c_base_element;
		class c_base_form;
		class c_stylesheet;
	}

	namespace drawing
	{
		struct font_t;

		using custom_font_id_t = std::size_t;

		using E_ROUNDING_FLAGS = std::uint8_t;
		enum E_ROUNDING_FLAGS_ : std::uint8_t
		{
			ROUNDFLAG_NONE = (1 << 0),
			ROUNDFLAG_TOP = (1 << 1),
			ROUNDFLAG_BOTTOM = (1 << 2),
			ROUNDFLAG_LEFT = (1 << 3),
			ROUNDFLAG_RIGHT = (1 << 4),
			ROUNDFLAG_ALL = ROUNDFLAG_TOP | ROUNDFLAG_BOTTOM | ROUNDFLAG_LEFT | ROUNDFLAG_RIGHT
		};

		inline std::shared_ptr<elements::c_base_form> m_target_form = nullptr;

		inline std::vector<std::shared_ptr<elements::c_base_form>> m_forms = { };

		inline std::map<std::string, std::shared_ptr<elements::c_stylesheet>> m_stylesheets = { };

		inline std::string m_stylesheet_key = "";

		inline std::vector<font_t*> m_custom_fonts = { };

		inline std::unordered_map<std::uint32_t, c_texture*> m_precached_texture = { };

		std::vector<std::shared_ptr<elements::c_base_form>>& get_forms();

		void push_form(std::shared_ptr<elements::c_base_form> form);

		void push_stylesheet(const std::string& key, std::shared_ptr<elements::c_stylesheet> stylesheet);

		std::shared_ptr<elements::c_stylesheet> get_stylesheet(const std::string& key);

		std::shared_ptr<elements::c_stylesheet> get_stylesheet();

		void set_stylesheet(const std::string& key);

		void draw_element(std::shared_ptr<elements::c_base_element> element);

		void draw_form(std::shared_ptr<elements::c_base_form> form);

		void draw_forms();

		custom_font_id_t register_custom_font(font_t* font);

		font_t* get_custom_font(const custom_font_id_t id);

		std::vector<font_t*>& get_custom_fonts();

		bool is_custom_font_id_valid(const custom_font_id_t id);

		custom_font_id_t get_max_font_id();

		void clean_up();

		inline void precache_texture(const std::uint32_t hash, c_texture* texture)
		{
			m_precached_texture[hash] = texture;
		}

		inline void precache_texture(const std::uint32_t hash, IDirect3DTexture9* texture_d3d, const math::point_t& size)
		{
			precache_texture(hash, new c_texture(texture_d3d, size));
		}

		inline bool get_is_texture_precached(const std::uint32_t hash)
		{
			return m_precached_texture.contains(hash);
		}

		inline c_texture* get_texture_precached(const std::uint32_t hash)
		{
			if (!get_is_texture_precached(hash))
				return nullptr;

			return m_precached_texture[hash];
		}

		inline void delete_cached_texture(const std::uint32_t hash)
		{
			if (!get_is_texture_precached(hash))
				return;

			if (m_precached_texture[hash])
			    delete m_precached_texture[hash];

			if (m_precached_texture.contains(hash))
				m_precached_texture.erase(hash);
		}

		namespace templates
		{
			using set_clip_t = std::add_pointer_t<void(shapes::rect_t rect, const bool force_override)>;
			using get_clip_t = std::add_pointer_t<shapes::rect_t()>;
			using pop_clip_t = std::add_pointer_t<void()>;
			using draw_rectangle_t = std::add_pointer_t<void(const shapes::rect_t& rect, const color_t& color, const float rounding, E_ROUNDING_FLAGS rounding_flags)>;
			using draw_rectangle_gradient_t = std::add_pointer_t<void(const shapes::rect_t& rect, const color_t& color1, const color_t& color2, const bool horizontal, const float rounding, E_ROUNDING_FLAGS rounding_flags)>;
			using draw_image_t = std::add_pointer_t<void(const shapes::rect_t& rect, void* texture, const color_t& color)>;
			using draw_image_rounded_t = std::add_pointer_t<void(const shapes::rect_t& rect, void* texture, const float rounding, const color_t& color)>;
			using draw_circle_t = std::add_pointer_t<void(const math::point_t& position, int r, const color_t& color, const bool fill)>;
			using draw_hollow_circle_t = std::add_pointer_t<void(const math::point_t& position, int r_in, int r_out, const color_t& color, const color_t& color2, const float progress_start, const float progress_end)>;

			using path_to_t = std::add_pointer_t< void(const math::point_t& point)>;
			using path_end_t = std::add_pointer_t<void()>;
			using path_stroke_t = std::add_pointer_t<void(const color_t& color, const bool closed, const int thickness)>;
			using path_fill_t = std::add_pointer_t<void(const color_t& color)>;

			using set_rotation_t = std::add_pointer_t<void(const float deg)>;
			using get_rotation_t = std::add_pointer_t<float()>;
			using pop_rotation_t = std::add_pointer_t<void()>;

			using draw_line_t = std::add_pointer_t<void(const math::point_t& point_1, const math::point_t& point2, const color_t& color, const int width)>;
			using create_font_t = std::add_pointer_t<void* (const std::string& type_name, const std::size_t height, const std::size_t weight, const std::uint8_t setup_flags)>;
			using create_font_memory_t = std::add_pointer_t<void* (const std::string& type_name, const std::size_t height, const std::size_t weight, const std::uint8_t setup_flags, const std::vector<std::uint8_t>& data)>;
			using register_font_t = std::add_pointer_t<void(font_t* font)>;
			using get_screen_resolution_t = std::add_pointer_t<math::point_t()>;
			using create_texture_rgba_t = std::add_pointer_t<void* (const color_t* data, const math::point_t& size)>;
			using create_texture_file_svg_t = std::add_pointer_t<void* (const std::string& path, math::point_t* size)>;
			using create_texture_file_svg_memory_t = std::add_pointer_t<void* (const std::string& data, math::point_t* size)>;
			using create_texture_file_t = std::add_pointer_t<void* (const std::string& path)>;
			using create_texture_file_memory_t = std::add_pointer_t<void* (const std::string& data)>;
			template<typename T> using draw_text_t = std::add_pointer_t<void(const math::point_t& position, const color_t& color, const T& text, void* font_ptr, const int pixel_size)>;
			template<typename T> using draw_text_cut_t = std::add_pointer_t<void(const math::point_t& position, const int x, const color_t& color, const T& text, void* font_ptr, const int pixel_size)>;
			template<typename T> using get_text_size_t = std::add_pointer_t<math::point_t(const T& text, void* font_ptr, const int pixel_size)>;
			template<typename T> using get_text_size_cut_t = std::add_pointer_t<math::point_t(const T& text, const int x, void* font_ptr, const int pixel_size)>;
		}

		inline templates::set_clip_t set_clip_ = nullptr;
		inline templates::pop_clip_t pop_clip = nullptr;
		inline templates::get_clip_t get_clip = nullptr;

		inline templates::set_rotation_t set_rotation = nullptr;
		inline templates::get_rotation_t get_rotation = nullptr;
		inline templates::pop_rotation_t pop_rotation = nullptr;

		inline templates::draw_rectangle_t draw_rectangle_ = nullptr;
		inline templates::draw_rectangle_t draw_rectangle_filled_ = nullptr;
		inline templates::draw_rectangle_gradient_t draw_rectangle_gradient_ = nullptr;
		inline templates::draw_image_t draw_image_ = nullptr;
		inline templates::draw_image_rounded_t draw_image_rounded_ = nullptr;
		inline templates::draw_circle_t draw_circle = nullptr;
		inline templates::draw_hollow_circle_t draw_hollow_circle = nullptr;
		inline templates::draw_line_t draw_line_ = nullptr;

		inline templates::path_to_t path_to = nullptr;
		inline templates::path_end_t path_end = nullptr;
		inline templates::path_stroke_t path_stroke = nullptr;
		inline templates::path_fill_t path_fill = nullptr;

		template<typename T> inline templates::draw_text_t<T> draw_text_ = nullptr;
		template<typename T> inline templates::draw_text_cut_t<T> draw_text_cut_ = nullptr;
		template<typename T> inline templates::get_text_size_t<T> get_text_size = nullptr;
		template<typename T> inline templates::get_text_size_cut_t<T> get_text_size_cut_ = nullptr;

		inline templates::create_texture_rgba_t create_texture_rgba = nullptr;
		inline templates::create_texture_file_svg_t create_texture_file_svg = nullptr;
		inline templates::create_texture_file_svg_memory_t create_texture_file_svg_memory = nullptr;
		inline templates::create_texture_file_t create_texture_file = nullptr;
		inline templates::create_texture_file_memory_t create_texture_file_memory = nullptr;

		/*inline templates::draw_text_t<char*> draw_text_;
		inline templates::draw_text_limited_t<char*> draw_text_cut_;
		inline templates::get_text_size_t<char*> get_text_size;
		inline templates::get_text_size_cut_t<char*> get_text_size_cut;*/

		inline templates::create_font_t create_font = nullptr;
		inline templates::create_font_memory_t create_font_memory = nullptr;

		inline templates::get_screen_resolution_t get_screen_resolution = nullptr;

		inline void draw_rectangle_filled(const shapes::rect_t& rect, const color_t& color, const float rounding = 0.f, const E_ROUNDING_FLAGS rounding_flags = ROUNDFLAG_ALL)
		{
			draw_rectangle_filled_(rect, color, rounding, rounding_flags);
		}

		inline void draw_rectangle_filled(const shapes::rect_t& rect, const color_t& color1, const color_t& color2, const bool horizontal, const float rounding = 0.f, const E_ROUNDING_FLAGS rounding_flags = ROUNDFLAG_ALL)
		{
			draw_rectangle_gradient_(rect, color1, color2, horizontal, rounding, rounding_flags);
		}

		inline void draw_rectangle(const shapes::rect_t& rect, const color_t& color, const float rounding = 0.f, const E_ROUNDING_FLAGS rounding_flags = ROUNDFLAG_ALL)
		{
			draw_rectangle_(rect, color, rounding, rounding_flags);
		}

		inline void draw_image_raw(const shapes::rect_t& rect, void* texture, const color_t& color = color_t::white())
		{
			draw_image_(rect, texture, color);
		}

		inline void draw_image_raw(const shapes::rect_t& rect, void* texture, const float rounding, const color_t& color = color_t::white())
		{
			draw_image_rounded_(rect, texture, rounding, color);
		}

		inline void draw_image(const math::point_t& position, c_texture* texture, const color_t& color = color_t::white())
		{
			draw_image_({ position, texture->get_size() }, texture->is_valid() ? texture->get_texture_ptr() : nullptr, color);
		}

		inline void draw_image(const math::point_t& position, c_texture* texture, const float rounding, const color_t& color = color_t::white())
		{
			draw_image_rounded_({ position, texture->get_size() }, texture->is_valid() ? texture->get_texture_ptr() : nullptr, rounding, color);
		}

		inline void draw_image(const shapes::rect_t& rect, c_texture* texture, const color_t& color = color_t::white())
		{
			draw_image_(rect, texture->is_valid() ? texture->get_texture_ptr() : nullptr, color);
		}

		inline void draw_image(const shapes::rect_t& rect, c_texture* texture, const float rounding, const color_t& color = color_t::white())
		{
			draw_image_rounded_(rect, texture->is_valid() ? texture->get_texture_ptr() : nullptr, rounding, color);
		}

		inline void draw_line(const math::point_t& point_1, const math::point_t& point_2, const color_t& color, const int width = 1)
		{
			draw_line_(point_1, point_2, color, width);
		}

		inline void draw_checkmark(const math::point_t& position, const math::point_t& size, const color_t& color, const float thickness = 2.f)
		{
			const float sz = size.x - thickness * 0.5f;
			math::point_t pos = position + (thickness * 0.3f, thickness * 0.35f);

			float third = sz / 3.0f;
			float bx = pos.x + third;
			float by = pos.y + sz - third * 0.5f;
			drawing::draw_line({ bx - third, by - third }, { bx, by }, color, thickness);
			drawing::draw_line({ bx, by }, { bx + third * 2.0f, by - third * 2.0f, }, color, thickness);
		}

		inline void draw_scrollbar(const float viewport, const float max_scroll, const float scroll, const shapes::rect_t& shaft_area, const float rounding, const bool horizontal, const color_t& shaft_color = { 51, 51, 51 }, const color_t& grip_color = { 102, 102, 102 })
		{
			const float area_delta = max_scroll - viewport;

			//-(verticalScrollbar.grip.y / verticalScrollbar.height) * content.height

			if (area_delta > 0)
			{
				drawing::draw_rectangle_filled(shaft_area, shaft_color, rounding);

				const float grip_height = (viewport / std::max(1.f, static_cast<float>(max_scroll))) * shaft_area.size.y;
				const float fill = ((horizontal ? shaft_area.size.x : shaft_area.size.y - grip_height) * (scroll) / (area_delta));

				drawing::draw_rectangle_filled({ { shaft_area.position.x + (horizontal ? fill : 0), shaft_area.position.y + (horizontal ? 0 : fill) }, { horizontal ? grip_height : shaft_area.size.x, horizontal ? shaft_area.size.y : grip_height} }, grip_color, rounding);
			}
			//else
				//drawing::draw_rectangle_filled(shaft_area, { 102, 102, 102 }, rounding);
		}

		inline void set_clip(const shapes::rect_t& rect, const bool force_override = false)
		{
			set_clip_(rect, force_override);
		}
	}
}
#endif
#endif