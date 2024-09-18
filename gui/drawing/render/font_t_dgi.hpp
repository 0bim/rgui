#pragma once
#ifndef TEXT_FT

#ifndef FONT_T_HPP
#define FONT_T_HPP

#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <d3d9.h>
#include <cstdio>

#include "../../helpers/math/point_t.hpp"

#include "../../helpers/utilities/stb_truetype.hpp"

namespace rgui
{
	namespace drawing
	{
		void test_ft();

		struct color_t;

		enum E_TEXTFLAGS : std::uint8_t
		{
			TEXTFLAG_NONE = (1 << 0),
			TEXTFLAG_CENTERED_X = (1 << 1),
			TEXTFLAG_CENTERED_Y = (1 << 2),
			TEXTFLAG_BOTTOM = (1 << 3),
			TEXTFLAG_LEFT = (1 << 4),
			TEXTFLAG_DROPSHADOW = (1 << 5),
			TEXTFLAG_OUTLINE = (1 << 6)
		};

		enum E_FONTFLAGS : std::uint8_t
		{
			FONTFLAG_NONE = (1 << 0),
			FONTFLAG_ANTIALIAS = (1 << 1),
			FONTFLAG_BOLD = (1 << 2),
			FONTFLAG_ITALIC = (1 << 3)
		};

#ifdef TEXT_FT
		struct glyph_t
		{
			int m_weight, m_height;
			IDirect3D9* m_texture;
		};
#endif

		/*please don't forget to pass this as a reference!!!. kind regards. -Raiin*/
		struct font_t
		{ 
			font_t();

			font_t(const char* type_name /*or string*/, const std::size_t height, const std::size_t weight, const std::uint8_t setup_flags);

			font_t(const char* type_name /*or string*/, const std::size_t height, const std::size_t weight, const std::uint8_t setup_flags, const std::vector<unsigned char>& data);
			
			~font_t()
			{
				unregister_font();
			}

			bool is_valid() const;

			void unregister_font();

			void setup();

			void setup(const std::vector<unsigned char>& data);

			int generate_character_textures(HDC ctx, const bool measure_only);

			template<typename T> 
			math::point_t get_text_size(const T& text, const std::uint8_t text_flags = TEXTFLAG_NONE);

			/*template<typename T>
			math::point_t get_text_size(const T* text, const std::uint8_t text_flags = TEXTFLAG_NONE);*/

			template<typename T>
			void draw(math::point_t position, const color_t& color, const T& text, const std::uint8_t text_flags = TEXTFLAG_NONE);

			/*template<typename T>
			void draw(math::point_t position, const color_t& color, const T* text, const std::uint8_t text_flags = TEXTFLAG_NONE);*/

			std::string m_type_name;
			std::size_t m_height, m_weight, m_id;
			std::uint8_t m_setup_flags;
			bool m_should_setup;

#ifdef TEXT_FT
			std::array<glyph_t, 256> m_glyphs = { };
#else
			float m_texure_coords[128 - 32][4];
			math::point_t m_texture_size;
			float m_text_scale;
			float m_char_spacing;
#endif

			//stbtt_fontinfo m_font_info;

			IDirect3DTexture9* m_texture;
		};

		using custom_font_id_t = std::size_t;
	}
}
#endif
#endif