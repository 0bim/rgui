#ifndef TEXT_FT

#include <shlobj_core.h>
#include <sstream>
#include <fstream>
#include "render.hpp"

#include "../../helpers/utilities/stb_image_write.hpp"

namespace rgui
{
	namespace drawing
	{
		void test_ft()
		{
			static IDirect3DTexture9* texture = nullptr;
			static UINT width, height;

			if (texture)
			{
				const shapes::rect_t rect = { { 200, 400 }, { width, height } };
				const color_t color = color_t::yellow();
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

				get_draw_list().push(vertices, D3DPT_TRIANGLELIST, get_clip(), texture, true);

				return;
			}

			static FT_Library  library;   /* handle to library     */
			static FT_Face     face;      /* handle to face object */

			if (FT_Init_FreeType(&library)) {
				std::cout << "error initializing lib" << std::endl;
			}

			if (FT_New_Face(library, TEXT("C:\\Users\\Raiin\\Downloads\\Montserrat-Regular.ttf"), 0, &face)) {
				std::cout << "font load failed" << std::endl;
			}
			else {
				//NHelper::OutputDebugStringN("font faces: %d \n", face->num_faces);
			}

			static int error;
			static int mTtfSize = 14;
			//static int mTtfResolution = 96;

			// size settings (convert font size to *64)
			FT_F26Dot6 ftSize = (FT_F26Dot6)(mTtfSize * (1 << 6));
			//error = FT_Set_Char_Size(face, ftSize, 0, 96, 0);

			error = FT_Set_Pixel_Sizes(face, mTtfSize + int(mTtfSize / 2.5), mTtfSize + int(mTtfSize / 2.5));

			// load glyph + render
			error = FT_Load_Char(face, L'a', FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL);
			if (error)
				std::cout << "failed to load char" << std::endl;

			// start copy procedure
			width = face->glyph->bitmap.width;
			height = face->glyph->bitmap.rows;
			const int pitch = face->glyph->bitmap.pitch;

			D3DXCreateTexture(get_device(), width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8, D3DPOOL_DEFAULT, &texture);

			D3DLOCKED_RECT lockedRect;
			texture->LockRect(0, &lockedRect, 0, 0);

			unsigned char* pSrcPixels = face->glyph->bitmap.buffer;
			unsigned char* pDestPixels = (unsigned char*)lockedRect.pBits;

			for (std::size_t i = 0; i < height; ++i)
			{
				std::memcpy(pDestPixels, pSrcPixels, width);

				//advance row pointers
				pSrcPixels += face->glyph->bitmap.pitch;
				pDestPixels += lockedRect.Pitch;
			}

			texture->UnlockRect(0);

			//int data_size = 0;
			//std::uint8_t* png_data = stbi_write_png_to_mem(pSrcPixels, width, width, height, 1, &data_size);

			//std::ofstream out(R"(C:\Users\Raiin\Desktop\coding\c++\hake-real\test.png)", std::ios::binary);
			//out.write((char*)png_data, data_size);
			//out.close();

			//texture = drawing::create_texture_file_memory(png_data, data_size);


			D3DXSaveTextureToFileA(R"(C:\Users\Raiin\Desktop\coding\c++\hake-real\test.png)", D3DXIFF_PNG, texture, 0);

			// release face
			FT_Done_Face(face);

			// library shutdown
			FT_Done_FreeType(library);
		}

		font_t::font_t()
			: m_type_name(""), m_height(12), m_weight(700), m_setup_flags(0), m_should_setup(true), m_char_spacing(0), m_texture(nullptr), m_text_scale(1.f)
		{
			setup();
			register_font(this);
		}

		font_t::font_t(const char* type_name /*or string*/, const std::size_t height, const std::size_t weight, const std::uint8_t setup_flags)
			: m_type_name(type_name), m_height(height), m_weight(weight), m_setup_flags(setup_flags), m_should_setup(true), m_char_spacing(0), m_texture(nullptr), m_text_scale(1.f)
		{
			setup();
			register_font(this);
		}

		font_t::font_t(const char* type_name /*or string*/, const std::size_t height, const std::size_t weight, const std::uint8_t setup_flags, const std::vector<unsigned char>& data)
			: m_type_name(type_name), m_height(height), m_weight(weight), m_setup_flags(setup_flags), m_should_setup(true), m_char_spacing(0), m_texture(nullptr), m_text_scale(1.f)
		{
			setup(data);
			register_font(this);
		}

		void create_gdi_font(HDC ctx, HGDIOBJ* gdi_font, const char* family, const int height, const int weight, const float scale, std::uint8_t flags)
		{
			int character_height = -MulDiv(height * 0.75f, static_cast<int>(GetDeviceCaps(ctx, LOGPIXELSY) * scale), 72);

			int italic = (flags & FONTFLAG_ITALIC) ? TRUE : FALSE;

			*gdi_font = CreateFont(character_height, 0, 0, 0, weight, italic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS, flags & FONTFLAG_ANTIALIAS ? ANTIALIASED_QUALITY : DEFAULT_QUALITY, VARIABLE_PITCH, family);
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
			if (m_should_setup && is_valid())
			{
				HDC gdi_ctx = nullptr;

				HGDIOBJ gdi_font = nullptr;
				HGDIOBJ prev_gdi_font = nullptr;
				HBITMAP bitmap = nullptr;
				HGDIOBJ prev_bitmap = nullptr;

				gdi_ctx = CreateCompatibleDC(nullptr);
				SetMapMode(gdi_ctx, MM_TEXT);

				create_gdi_font(gdi_ctx, &gdi_font, m_type_name.data(), m_height, m_weight, 1.f, m_setup_flags);

				prev_gdi_font = SelectObject(gdi_ctx, gdi_font);

				m_texture_size = { 128, 128 };

				HRESULT hr = S_OK;
				while (D3DERR_MOREDATA == (hr = generate_character_textures(gdi_ctx, true)))
					m_texture_size *= 2;

				D3DCAPS9 d3dCaps;
				get_device()->GetDeviceCaps(&d3dCaps);

				if (m_texture_size.x > static_cast<long>(d3dCaps.MaxTextureWidth))
				{
					m_text_scale = static_cast<float>(d3dCaps.MaxTextureWidth) / m_texture_size.x;
					m_texture_size.x = m_texture_size.y = d3dCaps.MaxTextureWidth;

					bool first_iteration = true;

					do
					{
						if (!first_iteration)
							m_text_scale *= 0.9f;

						DeleteObject(SelectObject(gdi_ctx, prev_gdi_font));

						create_gdi_font(gdi_ctx, &gdi_font, m_type_name.data(), m_height, m_weight, m_text_scale, m_setup_flags);

						prev_gdi_font = SelectObject(gdi_ctx, gdi_font);

						first_iteration = false;
					} while (D3DERR_MOREDATA == (hr = generate_character_textures(gdi_ctx, true)));
				}

				D3DXCreateTexture(get_device(), m_texture_size.x, m_texture_size.y, 1, D3DUSAGE_DYNAMIC, D3DFMT_A4R4G4B4, D3DPOOL_DEFAULT, &m_texture);

				DWORD* bitmap_bits;

				BITMAPINFO bitmap_ctx{};
				bitmap_ctx.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bitmap_ctx.bmiHeader.biWidth = m_texture_size.x;
				bitmap_ctx.bmiHeader.biHeight = -m_texture_size.y;
				bitmap_ctx.bmiHeader.biPlanes = 1;
				bitmap_ctx.bmiHeader.biCompression = BI_RGB;
				bitmap_ctx.bmiHeader.biBitCount = 32;

				bitmap = CreateDIBSection(gdi_ctx, &bitmap_ctx, DIB_RGB_COLORS, reinterpret_cast<void**>(&bitmap_bits), nullptr, 0);

				prev_bitmap = SelectObject(gdi_ctx, bitmap);

				SetTextColor(gdi_ctx, RGB(255, 255, 255));
				SetBkColor(gdi_ctx, 0x00000000);
				SetTextAlign(gdi_ctx, TA_TOP);

				generate_character_textures(gdi_ctx, false);

				D3DLOCKED_RECT locked_rect;
				m_texture->LockRect(0, &locked_rect, nullptr, 0);

				std::uint8_t* dst_row = static_cast<std::uint8_t*>(locked_rect.pBits);
				BYTE alpha;

				for (long y = 0; y < m_texture_size.y; y++)
				{
					std::uint16_t* dst = reinterpret_cast<std::uint16_t*>(dst_row);
					for (long x = 0; x < m_texture_size.x; x++)
					{
						alpha = ((bitmap_bits[m_texture_size.x * y + x] & 0xff) >> 4);
						if (alpha > 0)
						{
							*dst++ = ((alpha << 12) | 0x0fff);
						}
						else
						{
							*dst++ = 0x0000;
						}
					}
					dst_row += locked_rect.Pitch;
				}

				if (m_texture)
					m_texture->UnlockRect(0);

				SelectObject(gdi_ctx, prev_bitmap);
				SelectObject(gdi_ctx, prev_gdi_font);
				DeleteObject(bitmap);
				DeleteObject(gdi_font);
				DeleteDC(gdi_ctx);

				m_should_setup = false;
			}
		}

		void font_t::setup(const const std::vector<unsigned char>& data)
		{
			if (m_should_setup && is_valid())
			{
				DWORD n_fonts = 0;
				AddFontMemResourceEx(const_cast<unsigned char*>(data.data()), data.size(), nullptr, &n_fonts);
				setup();
			}
		}

		int font_t::generate_character_textures(HDC ctx, const bool measure_only)
		{
			SIZE size;
			char chr[2] = "x";

			if (0 == GetTextExtentPoint32(ctx, chr, 1, &size))
				return E_FAIL;

			m_char_spacing = static_cast<int>(std::ceil(size.cy * 0.3f));

			long x = m_char_spacing;
			long y = 0;

			for (char c = 32; c < 127; c++)
			{
				chr[0] = c;
				if (0 == GetTextExtentPoint32(ctx, chr, 1, &size))
					return E_FAIL ;

				if (x + size.cx + m_char_spacing > m_texture_size.x)
				{
					x = m_char_spacing;
					y += size.cy + 1;
				}

				if (y + size.cy > m_texture_size.y)
					return D3DERR_MOREDATA;

				if (!measure_only)
				{
					if (0 == ExtTextOut(ctx, x + 0, y + 0, ETO_OPAQUE, nullptr, chr, 1, nullptr))
						return E_FAIL;

					m_texure_coords[c - 32][0] = (static_cast<float>(x + 0 - m_char_spacing)) / m_texture_size.x;
					m_texure_coords[c - 32][1] = (static_cast<float>(y + 0 + 0)) / m_texture_size.y;
					m_texure_coords[c - 32][2] = (static_cast<float>(x + size.cx + m_char_spacing)) / m_texture_size.x;
					m_texure_coords[c - 32][3] = (static_cast<float>(y + size.cy + 0)) / m_texture_size.y;
				}

				x += size.cx + (2 * m_char_spacing);
			}

			return S_OK;
		}

		template<typename T>
		math::point_t font_t::get_text_size(const T& text, const std::uint8_t text_flags)
		{
			float row_width = 0.f;
			float row_height = (m_texure_coords[0][3] - m_texure_coords[0][1]) * m_texture_size.y;
			float width = 0.f;
			float height = row_height;

			if (text.size() == std::string::npos)
				return { };

			for (const auto& c : text)
			{
				if (!c)
					continue;

				if (c == '\n')
				{
					row_width = 0.f;
					height += row_height;
					continue;
				}

				if (c < ' ')
					continue;

				float tx1 = m_texure_coords[c - 32][0];
				float tx2 = m_texure_coords[c - 32][2];

				row_width += (tx2 - tx1) * m_texture_size.x - 2.f * m_char_spacing;

				if (row_width > width)
					width = row_width;
			}

			return { width, height };
		}

		/*template<typename T>
		math::point_t font_t::get_text_size(const T* text, const std::uint8_t text_flags)
		{
			return get_text_size(std::string(text), text_flags);
		}*/

		template<typename T>
		void font_t::draw(math::point_t position, const color_t& color, const T& text, const std::uint8_t text_flags)
		{
			if (is_valid())
			{
				setup(); /*no need for a check here becaues there's one insdie the function*/

				if (text.empty()) //why?
					return;

				std::size_t num_to_skip = 0;

				const math::point_t text_size = get_text_size(text, text_flags);

				if (text_flags & TEXTFLAG_CENTERED_X)
					position.x -= text_size.x * 0.5f;
				else if (text_flags & TEXTFLAG_LEFT)
					position.x -= text_size.x;
				if (text_flags & TEXTFLAG_CENTERED_Y)
					position.y -= text_size.y * 0.5f;
				else if (text_flags & TEXTFLAG_BOTTOM)
					position.y -= text_size.y;

				position.x -= m_char_spacing;

				const static std::array<char, 13> straight_line_char = { 'B', 'D', 'E', 'F', 'K', 'L', 'N', 'M', 'H', 'R','P', '[', ']' };

				//ghetto fix but if it works it works I guess
				if (std::find(straight_line_char.begin(), straight_line_char.end(), text[0]) != straight_line_char.end())
					position.x -= 1;

				float start_x = position.x;

				for (std::size_t i = 0; i < text.size(); i++)
				{
					if (num_to_skip > 0 && num_to_skip-- > 0)
						continue;

					/*if (flags & TEXTFLAG_COLORTAGS && c == '{') // format: {#aarrggbb} or {##rrggbb}, {#aarrggbb} will inherit alpha from color argument.
					{
						std::size_t index = &c - &text[0];
						if (std::size(text) > index + 11)
						{
							std::string color_str = text.substr(index, 11);
							if (color_str[1] == '#')
							{
								bool alpha = false;
								if ((alpha = color_str[10] == '}') || color_str[8] == '}')
								{
									num_to_skip += alpha ? 10 : 8;
									color_str.erase(std::remove_if(std::begin(color_str), std::end(color_str), [](char c) { return !std::isalnum(c); }), std::end(color_str));
									color = std::stoul(alpha ? color_str : "ff" + color_str, nullptr, 16);
									continue;
								}
							}
						}
					}*/

					const char c = text[i];

					if (c == '\n')
					{
						position.x = start_x;
						position.y += (m_texure_coords[0][3] - m_texure_coords[0][1]) * m_texture_size.y;
					}

					if (c < ' ')
						continue;

					const float tx1 = m_texure_coords[c - 32][0];
					const float ty1 = m_texure_coords[c - 32][1];
					const float tx2 = m_texure_coords[c - 32][2];
					const float ty2 = m_texure_coords[c - 32][3];

					const float w = (tx2 - tx1) * m_texture_size.x / m_text_scale;
					const float h = (ty2 - ty1) * m_texture_size.y / m_text_scale;

					const math::point_t scaled_size =  { w, h };

					const bool stay_in_pixel = c == '.' /*|| text[i + 1] == '.'*/;
					const float vetex_pad = stay_in_pixel ? 0.f : 0.5f;
					if (c != ' ')
					{
						const shapes::rect_t rect = { position, scaled_size };
						std::vector<math::vertex_t> vertices =
						{
							//left triangle
							{ { rect.points.left - vetex_pad, rect.points.top - vetex_pad }, color, { tx1, ty1 } },
							{ { rect.points.right - vetex_pad, rect.points.top - vetex_pad }, color, { tx2, ty1 } },
							{ { rect.points.left - vetex_pad, rect.points.bottom - vetex_pad }, color, { tx1, ty2 } },

							//right triangle
							{ { rect.points.right - vetex_pad, rect.points.top - vetex_pad }, color, { tx2, ty1 } },
							{ { rect.points.right - vetex_pad, rect.points.bottom - vetex_pad }, color, { tx2, ty2 } },
							{ { rect.points.left - vetex_pad, rect.points.bottom - vetex_pad }, color, { tx1, ty2 } }
						};

						if (text_flags & TEXTFLAG_DROPSHADOW)
						{
							const static auto dropshadow_color = drawing::color_t::black().to_hex();

							{
								for (auto& vertex : vertices)
								{
									vertex.m_color = dropshadow_color;
									vertex.m_position.x++;
								}
								drawing::get_draw_list().push(vertices, D3DPT_TRIANGLELIST, drawing::get_clip(), m_texture);
							}
							{
								for (auto& vertex : vertices)
								{
									vertex.m_position.y++;
								}
								drawing::get_draw_list().push(vertices, D3DPT_TRIANGLELIST, drawing::get_clip(), m_texture);
							}
							{
								for (auto& vertex : vertices)
								{
									vertex.m_color = color.to_hex();
									vertex.m_position.x--;
									vertex.m_position.y--;
								}
								//drawing::get_draw_list().push(vertices, D3DPT_TRIANGLELIST, drawing::get_clip(), m_texture);
							}

							/*for (auto& vtx : vertices) { vtx.m_color = shadow_color; vtx.m_position.x += 1.f; }
							drawing::get_draw_list().push(vertices, D3DPT_TRIANGLELIST, drawing::get_clip(), m_texture);

							for (auto& vtx : vertices) { vtx.m_position.x -= 2.f; }
							drawing::get_draw_list().push(vertices, D3DPT_TRIANGLELIST, drawing::get_clip(), m_texture);

							for (auto& vtx : vertices) { vtx.m_position.x += 1.f; vtx.m_position.y += 1.f; }
							drawing::get_draw_list().push(vertices, D3DPT_TRIANGLELIST, drawing::get_clip(), m_texture);

							for (auto& vtx : vertices) { vtx.m_position.y -= 2.f; }
							drawing::get_draw_list().push(vertices, D3DPT_TRIANGLELIST, drawing::get_clip(), m_texture);

							for (auto& vtx : vertices) { vtx.m_color = color.to_hex(); vtx.m_position.y -= 1.f; }
							drawing::get_draw_list().push(vertices, D3DPT_TRIANGLELIST, drawing::get_clip(), m_texture);*/
						}
						if (text_flags & TEXTFLAG_OUTLINE)
						{
							{
								for (auto& vertex : vertices)
								{
									vertex.m_color = drawing::color_t::black().to_hex();
									vertex.m_position.x--;
									vertex.m_position.y--;
								}
								drawing::get_draw_list().push(vertices, D3DPT_TRIANGLELIST, drawing::get_clip(), m_texture);
							}
							{
								for (auto& vertex : vertices)
								{
									vertex.m_position.x += 2;
									vertex.m_position.y += 2;
								}
								drawing::get_draw_list().push(vertices, D3DPT_TRIANGLELIST, drawing::get_clip(), m_texture);
							}
							{
								for (auto& vertex : vertices)
								{
									vertex.m_position.x--;
								}
								drawing::get_draw_list().push(vertices, D3DPT_TRIANGLELIST, drawing::get_clip(), m_texture);
							}
							{
								for (auto& vertex : vertices)
								{
									vertex.m_position.x += 2;
									vertex.m_position.y -= 2;
								}
								drawing::get_draw_list().push(vertices, D3DPT_TRIANGLELIST, drawing::get_clip(), m_texture);

								for (auto& vertex : vertices)
								{
									vertex.m_color = color.to_hex();
									vertex.m_position.x += 2;
									vertex.m_position.y -= 2;
								}
								//drawing::get_draw_list().push(vertices, D3DPT_TRIANGLELIST, drawing::get_clip(), m_texture);
							}
						}
							/*if (c == '.' && m_setup_flags & FONTFLAG_ANTIALIAS)
								drawing::get_draw_list().push(math::vertex_t({ position.x, position.y }, drawing::color_t::white()), D3DPT_POINTLIST);
							else*/
							drawing::get_draw_list().push(vertices, D3DPT_TRIANGLELIST, drawing::get_clip(), m_texture);
					}

					position.x += scaled_size.x - (2 * m_char_spacing);
					if (stay_in_pixel)
						position.x += 1;
				}
			}
		}

		/*template<typename T>
		void font_t::draw(math::point_t position, const color_t& color, const T* text, const std::uint8_t text_flags)
		{
			draw(position, color, std::string(text), text_flags);
		}*/

		template void font_t::draw<std::string>(math::point_t position, const color_t& color, const std::string& text, const std::uint8_t text_flags);
		template void font_t::draw<std::wstring>(math::point_t position, const color_t& color, const std::wstring& text, const std::uint8_t text_flags);
		template void font_t::draw<std::string_view>(math::point_t position, const color_t& color, const std::string_view& text, const std::uint8_t text_flags);
		template void font_t::draw<std::wstring_view>(math::point_t position, const color_t& color, const std::wstring_view& text, const std::uint8_t text_flags);
		/*template void font_t::draw<char>(math::point_t position, const color_t& color, const char* text, const std::uint8_t text_flags);
		template void font_t::draw<wchar_t>(math::point_t position, const color_t& color, const const wchar_t* text, const std::uint8_t text_flags);*/

		template math::point_t font_t::get_text_size<std::string>(const std::string& text, const std::uint8_t text_flags);
		template math::point_t font_t::get_text_size<std::wstring>(const std::wstring& text, const std::uint8_t text_flags);
		/*template math::point_t font_t::get_text_size<char>(const char* text, const std::uint8_t text_flags);
		template math::point_t font_t::get_text_size<wchar_t>(const wchar_t* text, const std::uint8_t text_flags);*/
	}
}

#endif