#ifdef RENDERER_OLD
#include <typeinfo>
#include <algorithm>
#include <sstream>
#include <filesystem>

#include "render.hpp"
#include "../../elements/elements.hpp"
#define NANOSVG_IMPLEMENTATION
#include "../../helpers/utilities/nanosvg.hpp"
#define NANOSVGRAST_IMPLEMENTATION
#include "../../helpers/utilities/nanosvgrast.hpp"
#include "../../helpers/utilities/stb_image_write.hpp"


/*color_hsv_t start = { 0.f, 100.f, 100.f };
for (std::size_t i = 0; i < img.size() - sizeof(color_t); i += sizeof(color_t))
{
	const color_t old_color = { img[i], img[i + 1], img[i + 2], img[i + 3] };
	start.h += static_cast<float>(i) / static_cast<float>(img.size() / sizeof(color_t));
	const auto& new_color = start.to_rgb();

	if (old_color.r != 0 || old_color.g != 0 || old_color.b != 0 || old_color.a != 0)
	for (std::size_t j = 0; j < 4; j++)
		img[i + j] = new_color[j];
}*/

namespace rgui
{
	namespace drawing
	{
		constexpr int m_circle_resolution = 150;

		void c_render::set_device(IDirect3DDevice9* device)
		{
			m_device = device;
			//if (m_device)
				//m_device->CreateStateBlock(D3DSBT_PIXELSTATE, &m_state_block);
		}

		IDirect3DDevice9* c_render::get_device() const
		{
			return m_device;
		}

		void c_render::begin()
		{
			static bool once = false;
			if (!once)
			{
				//m_draw_list.reserve(1240, 208); //prevent unnecessary copying and destruction of memory
				once = true;
			}

			if (FAILED(m_device->CreateStateBlock(D3DSBT_PIXELSTATE, &m_state_block)))
				return;

			if (duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - m_frame_timestamp).count() > 0)
				m_frame_timestamp = std::chrono::system_clock::now();

			m_frame_count++;

			m_device->SetFVF(m_fvf);

			m_device->SetVertexShader(nullptr);
			m_device->SetPixelShader(nullptr);

			m_device->SetRenderState(D3DRS_LIGHTING, false);
			m_device->SetRenderState(D3DRS_FOGENABLE, false);
			m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

			m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
			m_device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
			m_device->SetRenderState(D3DRS_ZWRITEENABLE, false);
			m_device->SetRenderState(D3DRS_STENCILENABLE, false);

			m_device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
			m_device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, TRUE);

			m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			m_device->SetRenderState(D3DRS_ALPHATESTENABLE, false);
			m_device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
			m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_INVDESTALPHA);
			m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			m_device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);

			m_device->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
			m_device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

			m_device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			m_device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			m_device->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
			m_device->SetSamplerState(0, D3DSAMP_SRGBTEXTURE, 0);

			m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
			m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
			m_device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);

			set_clip({ { 0.f, 0.f }, get_screen_resolution() });
		}

		void c_render::end()
		{
			set_clip({ { 0.f, 0.f }, get_screen_resolution() });
			m_clip_records.clear(); //reset clip records
			m_draw_list.clear();
			if (m_state_block)
			{
				m_state_block->Apply();
				m_state_block->Release();
			}

			if (duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - m_frame_timestamp).count() > 0) //if a second passed
			{
				m_frame_rate = (m_frame_count * 0.5f) + (m_frame_rate * 0.5f);
				m_frame_count = 0;
			}
		}

		void c_render::draw()
		{
			//sanity checks
			if (m_draw_list.m_vertices.empty())
				return;

			/*std::transform(m_draw_list.m_vertices.begin(), m_draw_list.m_vertices.end(), m_draw_list.m_vertices.begin(), [&](const math::vertex_t& vertex) FLIP EVERYTHING alwl
				{
					math::point_t point = { vertex.m_position.x, vertex.m_position.y };
					return math::vertex_t(point.scale_ascpect_ratio(1.f), vertex.m_color, vertex.m_texture);
				});*/

			/*IDirect3DVertexBuffer9* vertex_buffer;
			m_device->CreateVertexBuffer(m_draw_list.m_vertices.size() * sizeof(math::vertex_t), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, m_fvf, D3DPOOL_DEFAULT, &vertex_buffer, nullptr);

			void* buff;
			vertex_buffer->Lock(0, 0, reinterpret_cast<void**>(&buff), D3DLOCK_DISCARD);

			std::memcpy(buff, m_draw_list.m_vertices.data(), sizeof(math::vertex_t) * m_draw_list.m_vertices.size());

			vertex_buffer->Unlock();

			m_device->SetStreamSource(0, m_vertex_buffer, 0, sizeof(math::vertex_t));*/

			std::size_t pos = 0; //respresents the position in the vertex buffer
			for (const batch_t& batch : m_draw_list.m_batches) //loop through all the batches each should represent a shape/figure
			{
				const auto& clip = batch.m_clip;
				RECT clip_new = { clip.points.left, clip.points.top, clip.points.right, clip.points.bottom };
				m_device->SetScissorRect(&clip_new);

				std::size_t primitive_count = batch.m_vertex_count;

				if (primitive_count > 0)
				{
					primitive_count /= (batch.m_primitive_type == D3DPT_TRIANGLELIST || batch.m_primitive_type == D3DPT_TRIANGLESTRIP ? 3 : 2);

					m_device->SetTextureStageState(0, D3DTSS_COLOROP, batch.m_is_text ? D3DTOP_SELECTARG1 : D3DTOP_MODULATE);


					if (FAILED(m_device->SetTexture(0, batch.m_texture)))
						continue;
						//std::cout << "SetTexture Failed" << std::endl;

					if (FAILED(m_device->DrawPrimitiveUP(batch.m_primitive_type, primitive_count, &m_draw_list.m_vertices.data()[pos], sizeof(math::vertex_t))))
						continue;
					//m_device->DrawPrimitive(batch.m_primitive_type, pos, primitive_count);
						//std::cout << "DrawPrimitiveUP Failed" << std::endl;

					pos += batch.m_vertex_count;
				}
			}

			//vertex_buffer->Release();
		}

		void c_render::set_screen_resolution(const math::point_t& resolution)
		{
			m_screen_resolution = resolution;
		}

		math::point_t c_render::get_screen_resolution()
		{
			return m_screen_resolution;
		}

		void c_render::set_clip(const shapes::rect_t& rect)
		{
			m_clip_records.push_back(rect);
		}

		void c_render::pop_clip()
		{
			const math::point_t& screen_size = get_screen_resolution();

			if (m_clip_records.empty())
				set_clip({ { 0.f, 0.f }, screen_size });
			else
				m_clip_records.pop_back();
		}

		shapes::rect_t c_render::get_clip()
		{
			const math::point_t& screen_size = get_screen_resolution(); //asdfasd
			const shapes::rect_t screen = { { 0, 0 }, screen_size };
			return m_clip_records.empty() ? screen : m_clip_records.back();
		}

		shapes::rect_t c_render::get_clip_active()
		{
			RECT backup;
			m_device->GetScissorRect(&backup);
			return { math::point_t(backup.left, backup.top), math::point_t(backup.right - backup.left, backup.bottom - backup.top) };
		}

		IDirect3DTexture9* c_render::create_texture_rgba(const color_t* data, const math::point_t& size)
		{
			if (!data)
				return nullptr;

			std::vector<std::uint8_t> bitmap_8(size.x * size.y * sizeof(color_t));
			std::memcpy(bitmap_8.data(), data, size.x * size.y * sizeof(color_t));

			int data_size = 0;
			std::uint8_t* png_data = stbi_write_png_to_mem(bitmap_8.data(), size.x * 4, size.x, size.y, 4, &data_size);

			return create_texture_file_memory(png_data, data_size);
		}

		IDirect3DTexture9* c_render::create_texture_file_memory(const std::uint8_t* data, const std::size_t size)
		{
			if (!data)
				return nullptr;

			IDirect3DTexture9* texture = nullptr;

			//D3DLOCKED_RECT locked_rect;
			if (FAILED(D3DXCreateTextureFromFileInMemory(m_device, data, size, &texture)))
				return nullptr;

			/*texture->LockRect(0, &locked_rect, nullptr, D3DLOCK_DISCARD);

			std::memcpy(locked_rect.pBits, data, size.x * size.y);

			texture->UnlockRect(0);*/

			return texture;
		}

		IDirect3DTexture9* c_render::create_texture_file(const std::string& path)
		{
			if (path.empty() || !std::filesystem::exists(path))
				return nullptr;

			IDirect3DTexture9* texture = nullptr;

			//D3DLOCKED_RECT locked_rect;
			if (FAILED(D3DXCreateTextureFromFile(m_device, path.data(), &texture)))
				return nullptr;

			/*texture->LockRect(0, &locked_rect, nullptr, D3DLOCK_DISCARD);

			std::memcpy(locked_rect.pBits, data, size.x * size.y);

			texture->UnlockRect(0);*/

			return texture;
		}

		IDirect3DTexture9* c_render::create_texture_file_svg_memory(std::uint8_t* data, math::point_t* size)
		{
			if (!data)
				return nullptr;

			NSVGimage* image = nullptr;
			NSVGrasterizer* rast = nullptr;

			image = nsvgParse(reinterpret_cast<char*>(data), "px", 96.f);
			if (!image)
				return nullptr;

			const math::point_t dimensions = { image->width, image->height };

			rast = nsvgCreateRasterizer();
			if (!rast)
				return nullptr;

			std::vector<std::uint8_t> img(dimensions.x * dimensions.y * 4);

			nsvgRasterize(rast, image, 0, 0, 1, img.data(), dimensions.x, dimensions.y, dimensions.x * 4);

			int data_size = 0;
			std::uint8_t* png_data = stbi_write_png_to_mem(img.data(), dimensions.x * 4, dimensions.x, dimensions.y, 4, &data_size);

			auto texture = create_texture_file_memory(png_data, data_size);

			if (size)
				*size = dimensions;

			nsvgDeleteRasterizer(rast);
			nsvgDelete(image);

			return texture;
		}

		IDirect3DTexture9* c_render::create_texture_file_svg(const std::string& path, math::point_t* size)
		{
			if (path.empty())
				return nullptr;

			NSVGimage* image = nullptr;
			NSVGrasterizer* rast = nullptr;

			image = nsvgParseFromFile(path.data(), "px", 96.f);
			if (!image)
				return nullptr;

			const math::point_t dimensions = { image->width, image->height };

			rast = nsvgCreateRasterizer();
			if (!rast)
				return nullptr;

			std::vector<std::uint8_t> img(dimensions.x * dimensions.y * 4);

			nsvgRasterize(rast, image, 0, 0, 1, img.data(), dimensions.x, dimensions.y, dimensions.x * 4);

			int data_size = 0;
			std::uint8_t* png_data = stbi_write_png_to_mem(img.data(), dimensions.x * 4, dimensions.x, dimensions.y, 4, &data_size);

			auto texture = create_texture_file_memory(png_data, data_size);

			if (size)
			    *size = dimensions;

			nsvgDeleteRasterizer(rast);
			nsvgDelete(image);

			return texture;
		}

		void c_render::precache_texture(const std::uint32_t hash, const c_texture& texture)
		{
			m_precached_texture[hash] = texture;
		}

		void c_render::precache_texture(const std::uint32_t hash, IDirect3DTexture9* texture_d3d, const math::point_t& size)
		{
			precache_texture(hash, { texture_d3d, size });
		}

		bool c_render::get_is_texture_precached(const std::uint32_t hash)
		{
			return m_precached_texture.contains(hash);
		}

		c_texture c_render::get_texture_precached(const std::uint32_t hash)
		{
			return m_precached_texture[hash];
		}

		void c_render::register_font(font_t* font)
		{
			m_managed_fonts.push_back(font);
		}

		void c_render::unregister_font(font_t* font)
		{
			m_managed_fonts.erase(std::remove(m_managed_fonts.begin(), m_managed_fonts.end(), font), m_managed_fonts.end());
		}

		custom_font_id_t c_render::register_custom_font(const font_t& font)
		{
			m_custom_fonts.push_back(font);
			return m_custom_fonts.size() - 1;
		}

		font_t& c_render::get_custom_font(const custom_font_id_t id)
		{
			return m_custom_fonts.at(std::min(id, m_custom_fonts.size() - 1));
		}

		std::vector<font_t>& c_render::get_custom_fonts()
		{
			return m_custom_fonts;
		}

		bool c_render::is_custom_font_id_valid(const custom_font_id_t id)
		{
			return m_custom_fonts.size() > id;
		}

		void c_render::on_reset()
		{
			for (auto& font : m_managed_fonts)
				font->m_should_setup = true;
		}

		void c_render::draw_pixel(const math::point_t& position, const color_t& color)
		{
			m_draw_list.push({ position, color }, D3DPT_POINTLIST, get_clip());
		}

		void c_render::wu_pixel(const math::point_t& position, const color_t& color, const float c)
		{
			int c255 = (int)(255.f * c);
			int rgb = color.to_hex();

			D3DCOLOR px_color = color_t::white().to_hex();

			int rb = (0xFF00FF & rgb) * c255 + (0xFF00FF & px_color) * (255 - c255);
			int ag = (0x00FF00 & rgb) * c255 + (0x00FF00 & px_color) * (255 - c255);
			D3DCOLOR color_processed = rb >> 8 & 0xFF00FF | ag >> 8 & 0x00FF00;

			m_draw_list.push({ position, color_processed }, D3DPT_POINTLIST, get_clip());
		}

		void c_render::draw_line(const math::point_t& pos1, const math::point_t& pos2, const color_t& color)
		{
			/*auto ipart = [](float x) -> int {return int(std::floor(x)); };
			auto round = [](float x) -> float {return std::round(x); };
			auto fpart = [](float x) -> float {return x - std::floor(x); };
			auto rfpart = [=](float x) -> float {return 1 - fpart(x); };

			int x0 = pos1.x, x1 = pos2.x, y0 = pos1.y, y1 = pos2.y;

			const bool steep = abs(y1 - y0) > abs(x1 - x0);
			if (steep) {
				std::swap(x0, y0);
				std::swap(x1, y1);
			}
			if (x0 > x1) {
				std::swap(x0, x1);
				std::swap(y0, y1);
			}

			const float dx = x1 - x0;
			const float dy = y1 - y0;
			const float gradient = (dx == 0) ? 1 : dy / dx;

			int xpx11;
			float intery;
			{
				const float xend = round(x0);
				const float yend = y0 + gradient * (xend - x0);
				const float xgap = rfpart(x0 + 0.5);
				xpx11 = int(xend);
				const int ypx11 = ipart(yend);
				if (steep) 
				{
					wu_pixel({ ypx11, xpx11 }, color, rfpart(yend) * xgap);
					wu_pixel({ ypx11 + 1, xpx11 }, color, fpart(yend) * xgap);
				}
				else
				{
					wu_pixel({ xpx11, ypx11 }, color, rfpart(yend) * xgap);
					wu_pixel({ xpx11, ypx11 + 1 }, color, fpart(yend) * xgap);
				}
				intery = yend + gradient;
			}

			int xpx12;
			{
				const float xend = round(x1);
				const float yend = y1 + gradient * (xend - x1);
				const float xgap = rfpart(x1 + 0.5);
				xpx12 = int(xend);
				const int ypx12 = ipart(yend);
				if (steep) 
				{
					wu_pixel({ ypx12, xpx12 }, color, rfpart(yend)* xgap);
					wu_pixel({ ypx12 + 1, xpx12 }, color, fpart(yend) * xgap);
				}
				else 
				{
					wu_pixel({ xpx12, ypx12 }, color, rfpart(yend) * xgap);
					wu_pixel({ xpx12, ypx12 + 1 }, color, fpart(yend) * xgap);
				}
			}

			if (steep)
			{
				for (int x = xpx11 + 1; x < xpx12; x++) {
					wu_pixel({ ipart(intery), x }, color, rfpart(intery));
					wu_pixel({ ipart(intery) + 1, x }, color, fpart(intery));
					intery += gradient;
				}
			}
			else
			{
				for (int x = xpx11 + 1; x < xpx12; x++) {
					wu_pixel({ x, ipart(intery) }, color, rfpart(intery));
					wu_pixel({ x, ipart(intery) + 1 }, color, fpart(intery));
					intery += gradient;
				}
			}*/

			const std::vector<math::vertex_t> vertices =
			{
				{ { pos1.x, pos1.y }, color },
				{ { pos2.x, pos2.y }, color }
			};

			m_draw_list.push(vertices, D3DPT_LINESTRIP, get_clip());
		}

		std::vector<math::vertex_t> generate_rounded_vertex_buffer(const shapes::rect_t& rect, const int radius, const color_t& color)
		{
			constexpr int count = 24;

			std::vector<math::vertex_t> vertices(count);

			math::point_t add = { 0, 0 };

			for (int i = 0; i < vertices.size(); i++)
			{
				//const float angle = (std::powf(std::numbers::pi_v<float>, 2.f) * (static_cast<float>(i) / (m_circle_resolution * 0.5f)));

				const float angle = (static_cast <float>(i) / count) * 6.28f - (6.28f / 16.f);

				vertices[i] = { { radius + rect.position.x + add.x + (radius * std::sinf(angle)), rect.size.y - radius + rect.position.y + add.y + (radius * std::cosf(angle)) }, color };

				if (i == 4)
					add.y = -rect.size.y + (radius * 2.f);
				else if (i == 10)
					add.x = rect.size.x - (radius * 2.f);
				else if (i == 16)
					add.y = 0.f;
				else if (i == 22)
					add.x = 0.f;
			}

			return vertices;
		}   
		void c_render::draw_rectangle_filled(const shapes::rect_t& rect, const color_t& color, const bool aa)
		{
			//m_draw_list.push(generate_rounded_vertex_buffer(rect, 4, color), D3DPT_TRIANGLEFAN, get_clip());

			const float pad = aa ? -0.5f : 0.f;
			const std::vector<math::vertex_t> vertices =
			{
				//left triangle
				{ { rect.points.left + pad, rect.points.top + pad }, color },
				{ { rect.points.right + pad, rect.points.top + pad }, color },
				{ { rect.points.left + pad, rect.points.bottom + pad }, color },

				//right triangle
				{ { rect.points.right + pad, rect.points.top + pad }, color },
				{ { rect.points.right + pad, rect.points.bottom + pad }, color },
				{ { rect.points.left + pad, rect.points.bottom + pad }, color }
			};

			m_draw_list.push(vertices, D3DPT_TRIANGLELIST, get_clip());
		}

		void c_render::draw_rectangle_filled(const shapes::rect_t& rect, const color_t& color, const color_t& color_gradient, const bool horizontal)
		{
			const float pad = 0.5f;
			const std::vector<math::vertex_t> vertices =
			{
				//left triangle
				{ { rect.points.left - pad, rect.points.top - pad }, color },
				{ { rect.points.right + pad, rect.points.top - pad }, horizontal ? color_gradient : color },
				{ { rect.points.left - pad, rect.points.bottom + pad },  horizontal ? color : color_gradient },

				//right triangle
				{ { rect.points.right + pad, rect.points.top - pad }, horizontal ? color_gradient : color },
				{ { rect.points.right + pad, rect.points.bottom + pad }, color_gradient },
				{ { rect.points.left - pad, rect.points.bottom + pad }, horizontal ? color : color_gradient }
			};

			m_draw_list.push(vertices, D3DPT_TRIANGLELIST, get_clip());
		}

		void c_render::draw_image(const shapes::rect_t& rect, IDirect3DTexture9* texture, const color_t& color)
		{
			const float pad = 0.5f;
			const std::vector<math::vertex_t> vertices =
			{
				//left triangle
				{ { rect.points.left - pad, rect.points.top - pad }, color, { 0.f, 0.f } },
				{ { rect.points.right + pad, rect.points.top - pad }, color, { 1.f, 0.f }},
				{ { rect.points.left - pad, rect.points.bottom + pad }, color, { 0.f, 1.f } },

				//right triangle
				{ { rect.points.right + pad, rect.points.top - pad }, color, { 1.f, 0.f } },
				{ { rect.points.right + pad, rect.points.bottom + pad }, color, { 1.f, 1.f } },
				{ { rect.points.left - pad, rect.points.bottom + pad }, color, { 0.f, 1.f } }
			};

			m_draw_list.push(vertices, D3DPT_TRIANGLELIST, get_clip(), texture);
		}

		void c_render::draw_image(const shapes::rect_t& rect, IDirect3DTexture9* texture)
		{
			draw_image(rect, texture, { });
		}

		void c_render::draw_image(const math::point_t& position, const c_texture& texture, const color_t& color)
		{
			//I can't use the other constructor due to coimpiler error
			const math::point_t& size = texture.get_size();
			auto rect = shapes::rect_t(position.x, position.y, position.x + size.x, position.y + size.y);
			draw_image(rect, texture.get_texture_ptr(), color);
		}

		void c_render::draw_image(const math::point_t& position, const c_texture& texture)
		{
			draw_image(position, texture, { });
		}

		void c_render::draw_rectangle(const shapes::rect_t& rect, const color_t& color)
		{
			draw_line({ rect.points.left, rect.points.top }, { rect.points.right, rect.points.top }, color);
			draw_line({ rect.points.left, rect.points.bottom }, { rect.points.right, rect.points.bottom }, color);
			draw_line({ rect.points.left, rect.points.top }, { rect.points.left, rect.points.bottom }, color);
			draw_line({ rect.points.right, rect.points.top }, { rect.points.right, rect.points.bottom + 1 }, color);

			/*draw_rectangle_filled({ { rect.points.left, rect.points.top }, { rect.size.x, 1 } }, color, false);
			draw_rectangle_filled({ { rect.points.left, rect.points.bottom }, { rect.size.x, 1 } }, color, false);
			draw_rectangle_filled({ { rect.points.left, rect.points.top }, { 1, rect.size.y } }, color, false);
			draw_rectangle_filled({ { rect.points.right, rect.points.top }, { 1, rect.size.y + 1 } }, color, false);*/
		}

		std::array<math::vector2d, m_circle_resolution> get_circle_coordinates(const int radius)
		{
			static std::unordered_map<int, std::array<math::vector2d, m_circle_resolution>> circle_radius_vertices = { };

			if (circle_radius_vertices.contains(radius))
			{
				for (int i = 0; i < m_circle_resolution; i++)
				{
					const math::vector2d& point = circle_radius_vertices[radius][i];
				}
			}
			else
			{
				for (int i = 0; i < m_circle_resolution; i++)
				{
					const float angle = (std::numbers::pi_v<float> * 2.f * (static_cast<float>(i) / (m_circle_resolution * 0.5f)));
					const float pad = i > m_circle_resolution / 2 ? 0.5f : -0.5f;
					const math::vector2d& point = { (radius * std::cosf(angle)) + pad , (radius * std::sinf(angle)) + pad };
					circle_radius_vertices[radius][i] = point;
				}
			}

			return circle_radius_vertices[radius];
		}

		void c_render::draw_circle(const math::point_t& position, const int radius, const c_texture& texture, const color_t& color)
		{
			const auto& cirlce_points = get_circle_coordinates(radius);
			std::vector<math::vertex_t> vertices(m_circle_resolution);

			for (std::size_t i = 0; i < m_circle_resolution; i++)
			{
				const auto& coordinates = cirlce_points[i];
				const auto& p = math::vector2d(position.x + coordinates.x, position.y + coordinates.y);
				const math::vector2d tex = { (coordinates.x / radius + 1.f) * 0.5f, (coordinates.y / radius + 1.f) * 0.5f };
				vertices[i] = { p, color, tex };
			}

			m_draw_list.push(vertices, D3DPT_TRIANGLEFAN, get_clip(), texture.get_texture_ptr());
		}

		void c_render::draw_circle(const math::point_t& position, const int radius, const color_t& color, const bool filled)
		{
			const auto& cirlce_points = get_circle_coordinates(radius);
			std::vector<math::vertex_t> vertices(m_circle_resolution);

			for (std::size_t i = 0; i < m_circle_resolution; i++)
			{
				const auto& coordinates = cirlce_points[i];
				vertices[i] = { { position.x + coordinates.x, position.y + coordinates.y }, color };
			}

			m_draw_list.push(vertices, filled ? D3DPT_TRIANGLEFAN : D3DPT_LINESTRIP, get_clip());
		}

		void c_render::draw_circle_annulus(const math::point_t& position, const int radius_in, const int radius_out, const float blend, const color_t& color)
		{
			constexpr int count = 200;
			std::vector<math::vertex_t> vertices(count);

			const static float tau = std::powf(std::numbers::pi_v<float>, 2.f); //std::numbers is missing this for some reason
			for (int i = 0; i < count - 2; i += 2)
			{
				const float angle = (tau * blend * (static_cast<float>(i) / (count * 0.5f)));
				const math::vector2d point_out = { (radius_in * std::cosf(angle)), (radius_in * std::sinf(angle)) };
				vertices[i] = { { position.x + point_out.x, position.y + point_out.y }, color };

				const math::vector2d point_in = { (radius_out * std::cosf(angle)), (radius_out * std::sinf(angle)) };
				vertices[i + 1] = { { position.x + point_in.x, position.y + point_in.y }, color };
			}

			/*for (int i = 1; i < count; i += 2)
			{
				const float angle = (std::numbers::pi_v<float> * (3.f * blend) * (static_cast<float>(i) / (count * 0.5f)));
				const math::vector2d point_out = { (radius_out * std::cosf(angle)), (radius_out * std::sinf(angle)) };
				vertices[i] = { { position.x + point_out.x, position.y + point_out.y }, color };
			}*/

			m_draw_list.push(vertices, D3DPT_TRIANGLESTRIP, get_clip());
		}

		void c_render::draw_element(std::shared_ptr<elements::c_base_element> element)
		{
			if (!element)
				return;

			element->update();

			if (!element->get_is_visible())
				return;

			element->draw();
		}

		void c_render::draw_form(std::shared_ptr<elements::c_base_form> form)
		{
			draw_element(form);
		}
	}
}
#else
#include "render.hpp"
#include "../../elements/elements.hpp"

namespace rgui
{
	namespace drawing
	{
		std::vector<std::shared_ptr<elements::c_base_form>>& get_forms()
		{
			return m_forms;
		}

		void push_form(std::shared_ptr<elements::c_base_form> form)
		{
			m_forms.push_back(form);
		}

		void push_stylesheet(const std::string& key, std::shared_ptr<elements::c_stylesheet> stylesheet)
		{
			m_stylesheets.insert({ key, stylesheet });
		}

		std::shared_ptr<elements::c_stylesheet> get_stylesheet(const std::string& key)
		{
			return m_stylesheets[key];
		}

		std::shared_ptr<elements::c_stylesheet> get_stylesheet()
		{
			return get_stylesheet(m_stylesheet_key);
		}

		void set_stylesheet(const std::string& key)
		{
			m_stylesheet_key = key;
		}

		void draw_element(std::shared_ptr<elements::c_base_element> element)
		{
			if (!element)
				return;

			element->update();

			if (!element->get_is_visible())
				return;

			element->draw();
		}

		void draw_form(std::shared_ptr<elements::c_base_form> form)
		{
			draw_element(form);
		}

		void draw_forms()
		{
			for (std::size_t i = 0; i < m_forms.size(); i++)
			{
				const auto& form = m_forms[i];
				if (form && form != m_target_form)
					draw_form(form);
			}

			if (m_target_form)
			    draw_form(m_target_form);
		}

		custom_font_id_t register_custom_font(font_t* font)
		{
			m_custom_fonts.push_back(font);
			return m_custom_fonts.size() - 1;
		}

		font_t* get_custom_font(const custom_font_id_t id)
		{
			return m_custom_fonts.at(std::min(id, m_custom_fonts.size() - 1));
		}

		std::vector<font_t*>& get_custom_fonts()
		{
			return m_custom_fonts;
		}

		bool is_custom_font_id_valid(const custom_font_id_t id)
		{
			return m_custom_fonts.size() > id;
		}

		custom_font_id_t get_max_font_id()
		{
			return m_custom_fonts.size();
		}

		void clean_up()
		{
			for (auto font : m_registered_fonts)
				delete font; 

			for (auto texture : m_precached_texture)
				delete texture.second;

			m_registered_fonts.clear();

			m_custom_fonts.clear();

			m_precached_texture.clear();
		}
	}
}

#endif