#pragma once
#ifndef FONT_T_HPP
#define FONT_T_HPP
#include <string>
#include <cstdint>
#include <vector>
#include "../../helpers/math/point_t.hpp"

namespace rgui
{
	namespace drawing
	{
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
			FONTFLAG_ITALIC = (1 << 2),
			FONTFLAG_DROPSHADOW = (1 << 3),
			FONTFLAG_OUTLINE = (1 << 4)
		};

		struct font_t
		{
			font_t();

			font_t(const std::string& type_name, const std::size_t height, std::size_t weight, const std::uint8_t setup_flags);

			font_t(const std::string& type_name, const std::size_t height, const std::size_t weight, const std::uint8_t setup_flags, std::vector<std::uint8_t>& data);

			~font_t();

			void setup();

			template<typename T>
			void draw(math::point_t position, const color_t& color, const T& text, const std::uint8_t flags = TEXTFLAG_NONE);

			template<typename T>
			void draw(math::point_t position, const int x, const color_t& color, const T& text, const std::uint8_t flags = TEXTFLAG_NONE);
		
			template<typename T>
			math::point_t get_text_size(const T& text);

			template<typename T>
			math::point_t get_text_size(const T& text, const int x);

			void* m_font_ptr;

			std::string m_type_name;
			std::size_t m_height, m_weight;
			std::uint8_t m_setup_flags;

			std::vector<std::uint8_t>* m_data;

			inline static bool m_can_setup_fonts = false;
			inline static bool m_reset_fonts = true;
		};

		inline std::vector<font_t*> m_registered_fonts = { };

		inline void register_font(font_t* font)
		{
			if (std::find(m_registered_fonts.begin(), m_registered_fonts.end(), font) == m_registered_fonts.end())
				m_registered_fonts.push_back(font);
		}

		inline void unregister_font(font_t* font)
		{
			if (auto it = std::find(m_registered_fonts.begin(), m_registered_fonts.end(), font); it != m_registered_fonts.end())
				m_registered_fonts.erase(it);
		}
	}
}

#endif