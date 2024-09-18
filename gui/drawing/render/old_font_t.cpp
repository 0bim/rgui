#include <sstream>
#include <fstream>

#include "font_t.hpp"

#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include FT_STROKER_H

#include "render.hpp"

#include "../../helpers/utilities/stb_image_write.hpp"

namespace rgui
{
	namespace drawing
	{
		font_t::font_t()
			: m_type_name(""), m_height(12), m_weight(700), m_setup_flags(0), m_should_setup(true), m_texture(nullptr), m_memory_data(nullptr)
		{
			setup();
			register_font(this);
		}

		font_t::font_t(const char* type_name /*or string*/, const std::size_t height, const std::size_t weight, const std::uint8_t setup_flags)
			: m_type_name(type_name), m_height(height), m_weight(weight), m_setup_flags(setup_flags), m_should_setup(true), m_texture(nullptr), m_memory_data(nullptr)
		{
			setup();
			register_font(this);
		}

		font_t::font_t(const char* type_name /*or string*/, const std::size_t height, const std::size_t weight, const std::uint8_t setup_flags, const std::vector<unsigned char>& data)
			: m_type_name(type_name), m_height(height), m_weight(weight), m_setup_flags(setup_flags), m_should_setup(true), m_texture(nullptr)
		{
			if (data.empty())
			{
				m_memory_data = nullptr;
				setup();
			}
			else
			{
				m_memory_data = &data;
				setup(data);
			}

			register_font(this);
		}

		bool font_t::is_valid() const
		{
			return !m_type_name.empty();// && is_font_id_valid(m_id);
		}

		void font_t::unregister_font()
		{

		}

		void font_t::setup()
		{
			if (m_memory_data)
				return setup(*m_memory_data);

			if (m_should_setup && is_valid())
			{
				std::string path = get_font_path(m_type_name);
#ifdef _DEBUG
				if (path.empty())
					path = get_font_path("Arial");
#endif
				if (!path.empty())
				{
					FT_Library library;
					if (!FT_Init_FreeType(&library))
					{
						FT_Face face;
						if (!FT_New_Face(library, path.data(), 0, &face));
						{
							generate_glyphs(library, face);

							FT_Done_Face(face);
						}
					}

					FT_Done_FreeType(library);

					m_should_setup = false;
				}
			}
		}

		void font_t::setup(const std::vector<unsigned char>& data)
		{
			if (m_should_setup && is_valid())
			{
				FT_Library library;
				if (!FT_Init_FreeType(&library))
				{
					FT_Face face;
					if (!FT_New_Memory_Face(library, data.data(), data.size(), 0, &face));
					{
						generate_glyphs(library, face);

						FT_Done_Face(face);
					}
				}

				FT_Done_FreeType(library);

				m_should_setup = false;
			}
		}

		void font_t::generate_glyph(const char c, FT_Library library, FT_Face face)
		{
			//static int res = 96;

			// size settings (convert font size to *64)
			int font_height_adjusted = m_height * 0.75f;
			//if (c == 'n')
				//font_height_adjusted -= 1;

			FT_F26Dot6 ft_size = (FT_F26Dot6)(font_height_adjusted * (1 << 6));
			FT_Set_Char_Size(face, ft_size, 0, 96, 96);

			//FT_Set_Pixel_Sizes(face, height + int(height / 2.5f), height + int(height / 2.5f));

			int extra_flags = FT_LOAD_RENDER | FT_LOAD_NO_BITMAP;

			if (c != 'i' && c != '.')
			   extra_flags |= FT_LOAD_FORCE_AUTOHINT;

			if (m_setup_flags & FONTFLAG_ANTIALIAS)
				extra_flags |= FT_LOAD_TARGET_NORMAL;

			// load glyph + render
			FT_Load_Char(face, c, extra_flags);

			//FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

			glyph_t& glyph_ref = m_glyphs[c];

			const int advance_extra = (m_weight > 400 ? (m_weight - 400) / 100 : 0); //((m_height >= 12 && m_weight > 400 && c != ' ' && c != '-' && c != '|') || c == 'M' || c == 'A') ? 1 : 0;

			if (m_weight > 400 && c != '"')
				FT_Bitmap_Embolden(library, &face->glyph->bitmap, static_cast<int>((face->size->metrics.x_ppem * m_weight) * (1.f / 64.f)), 0);

			FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL); //NOT NEEDED I THINK REMEMBER REMEMBER REMEMBER REMEMBER

			glyph_ref.m_size = { face->glyph->bitmap.width, face->glyph->bitmap.rows };
			glyph_ref.m_advance = { (face->glyph->advance.x >> 6) + advance_extra, (face->glyph->advance.y >> 6) + advance_extra };
			glyph_ref.m_bearing = { face->glyph->bitmap_left, face->glyph->bitmap_top };
			glyph_ref.m_new_line_spacing = std::max(static_cast<std::size_t>(face->size->metrics.height >> 6 + 1), m_height);

			const int pitch = face->glyph->bitmap.pitch;

			D3DXCreateTexture(get_device(), glyph_ref.m_size.x, glyph_ref.m_size.y, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8, D3DPOOL_DEFAULT, &glyph_ref.m_texture);

			D3DLOCKED_RECT lockedRect;
			glyph_ref.m_texture->LockRect(0, &lockedRect, 0, 0);

			unsigned char* src = face->glyph->bitmap.buffer;
			unsigned char* dest = (unsigned char*)lockedRect.pBits;

			for (std::size_t i = 0; i < glyph_ref.m_size.y; ++i)
			{
				std::memcpy(dest, src, glyph_ref.m_size.x);

				//advance row pointers
				src += face->glyph->bitmap.pitch;
				dest += lockedRect.Pitch;
			}

			glyph_ref.m_texture->UnlockRect(0);

			//int data_size = 0;
			//std::uint8_t* png_data = stbi_write_png_to_mem(face->glyph->bitmap.buffer, width, width, height, 1, &data_size);

			//std::ofstream out(R"(C:\Users\Raiin\Desktop\coding\c++\hake-real\test.png)", std::ios::binary);
			//out.write((char*)png_data, data_size);
			//out.close();

			//glyph_ref.m_texture = drawing::create_texture_file_memory(png_data, data_size);

			//this shit sux
			//D3DXSaveTextureToFile((std::string(R"(C:\Users\Raiin\Desktop\coding\c++\hake-real\characters\char_)") + std::to_string((int)c) + ".png").data(), D3DXIFF_PNG, glyph_ref.m_texture, 0);
		}

		template<typename T>
		math::point_t font_t::get_text_size(const T& text, const std::uint8_t text_flags, const float text_scale, const float x_limit)
		{
			math::point_t size = { 0, 0 };

			float row_width = 0.f;

			std::string text_drawn = "";

			for (auto c : text)
			{
				if (!(c > 0 && c < m_glyphs.size()))
					continue;

				glyph_t& glyph = m_glyphs[c];

				if (!glyph.m_texture)
					continue; //generate_glyph(c);

				if (c == '\n' || (row_width >= x_limit && c == ' ' && x_limit > 0.f))
				{
					row_width = 0.f;
					size.y += glyph.m_new_line_spacing;
					continue;
				}

				row_width += glyph.m_advance.x;

				if (size.y < glyph.m_size.y)
					size.y = glyph.m_size.y;

				if (row_width > size.x)
					size.x = row_width;

				text_drawn.push_back(c);
			}
			if (text_flags & TEXTFLAG_DROPSHADOW)
				size += { (size.y * 0.035f), (size.y * 0.035f) };

			return size * text_scale;
		}

		/*template<typename T>
		math::point_t font_t::get_text_size(const T* text, const std::uint8_t text_flags)
		{
			return get_text_size(std::string(text), text_flags);
		}*/

		template<typename T>
		void font_t::draw(math::point_t position, const color_t& color, const T& text, const std::uint8_t text_flags, const float text_scale, const float x_limit)
		{
			if (is_valid())
			{
				setup(); /*no need for a check here becaues there's one insdie the function*/

				if (text.empty()) //why?
					return;

				std::size_t num_to_skip = 0;

				const math::point_t& text_size = get_text_size(text, text_flags);

				if (text_flags & TEXTFLAG_CENTERED_X)
					position.x -= text_size.x * 0.5f * text_scale;
				else if (text_flags & TEXTFLAG_LEFT)
					position.x -= text_size.x * text_scale;
				if (text_flags & TEXTFLAG_CENTERED_Y)
					position.y -= text_size.y * 0.5f * text_scale;
				else if (text_flags & TEXTFLAG_BOTTOM)
					position.y -= text_size.y * text_scale;

				const int bearing_m_y = m_glyphs['M'].m_bearing.y;

				const int origin_x = position.x;

				std::string text_drawn = "";

				for (char c : text)
				{
					if (!(c > 0 && c < m_glyphs.size()))
						continue;

					const glyph_t& glyph = m_glyphs[c];

					/* no texture so default generated - non-ascii char or never used before */
					if (!glyph.m_texture)
						continue; //generate_glyph(c);

					/* No need to render space duhh */
					if (c != ' ')
					{
						const float pos_x = position.x + glyph.m_bearing.x * text_scale;
						const float pos_y = position.y + (bearing_m_y - glyph.m_bearing.y) * text_scale;

						const float w = glyph.m_size.x * text_scale;
						const float h = glyph.m_size.y * text_scale;

						const shapes::rect_t rect = { { pos_x, pos_y }, { w, h } };
						const float pad = 0.5f;
						const std::vector<math::vertex_t> vertices =
						{
							//left triangle
							{ { rect.points.left - pad, rect.points.top - pad }, color, { 0.f, 0.f } },
							{ { rect.points.right - pad, rect.points.top - pad }, color, { 1.f, 0.f }},
							{ { rect.points.left - pad, rect.points.bottom - pad }, color, { 0.f, 1.f } },

							//right triangle
							{ { rect.points.right - pad, rect.points.top - pad }, color, { 1.f, 0.f } },
							{ { rect.points.right - pad, rect.points.bottom - pad }, color, { 1.f, 1.f } },
							{ { rect.points.left - pad, rect.points.bottom - pad }, color, { 0.f, 1.f } }

							//left triangle
							//{ { rect.points.left - pad, rect.points.top + pad }, color, { 0.f, 0.f } },
							//{ { rect.points.left - pad, rect.points.bottom - pad }, color, { 0.f, 1.f } },
							//{ { rect.points.right + pad, rect.points.bottom - pad }, color, { 1.f, 1.f }},

							//right triangle
							//{ { rect.points.left - pad, rect.points.top + pad }, color, { 0.f, 0.f } },
							//{ { rect.points.right + pad, rect.points.top + pad }, color, { 1.f, 0.f } },
							//{ { rect.points.right + pad, rect.points.bottom - pad }, color, { 1.f, 1.f } }
						};

						get_draw_list().push(vertices, D3DPT_TRIANGLELIST, get_clip(), glyph.m_texture, true);
					}
					else
					{
						if (get_text_size(text_drawn).x >= x_limit && x_limit > 0.f)
						{
							position.x = origin_x;
							position.y += glyph.m_new_line_spacing;
							text_drawn = "";
							continue; //so it doesn't advance forward
						}
					}

					if (c == '\n')
					{
						position.x = origin_x;
						position.y += glyph.m_new_line_spacing;
					}
					else
					   position.x += glyph.m_advance.x * text_scale;

					text_drawn.push_back(c);
				}
			}
		}

		/*template<typename T>
		void font_t::draw(math::point_t position, const color_t& color, const T* text, const std::uint8_t text_flags)
		{
			draw(position, color, std::string(text), text_flags);
		}*/

		template void font_t::draw<std::string>(math::point_t position, const color_t& color, const std::string& text, const std::uint8_t text_flags, const float text_scale, const float x_limit);
		template void font_t::draw<std::wstring>(math::point_t position, const color_t& color, const std::wstring& text, const std::uint8_t text_flags, const float text_scale, const float x_limit);
		template void font_t::draw<std::string_view>(math::point_t position, const color_t& color, const std::string_view& text, const std::uint8_t text_flags, const float text_scale, const float x_limit);
		template void font_t::draw<std::wstring_view>(math::point_t position, const color_t& color, const std::wstring_view& text, const std::uint8_t text_flags, const float text_scale, const float x_limit);
		/*template void font_t::draw<char>(math::point_t position, const color_t& color, const char* text, const std::uint8_t text_flags);
		template void font_t::draw<wchar_t>(math::point_t position, const color_t& color, const const wchar_t* text, const std::uint8_t text_flags);*/

		template math::point_t font_t::get_text_size<std::string>(const std::string& text, const std::uint8_t text_flags, const float text_scale, const float x_limit);
		template math::point_t font_t::get_text_size<std::wstring>(const std::wstring& text, const std::uint8_t text_flags, const float text_scale, const float x_limit);
		/*template math::point_t font_t::get_text_size<char>(const char* text, const std::uint8_t text_flags);
		template math::point_t font_t::get_text_size<wchar_t>(const wchar_t* text, const std::uint8_t text_flags);*/
	}
}