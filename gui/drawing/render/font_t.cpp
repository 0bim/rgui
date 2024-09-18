#pragma once
#include "font_t.hpp"
#include "render.hpp"

namespace rgui
{
	namespace drawing
	{
		font_t::font_t()
			: m_type_name(""), m_height(12), m_weight(700), m_setup_flags(0), m_data(nullptr), m_font_ptr(nullptr)
		{
			assert(this != nullptr);
			setup();
			m_registered_fonts.push_back(this);
		}

		font_t::font_t(const std::string& type_name, const std::size_t height, std::size_t weight, const std::uint8_t setup_flags)
			: m_type_name(type_name), m_height(height), m_weight(weight), m_setup_flags(setup_flags), m_data(nullptr), m_font_ptr(nullptr)
		{
			setup();
			m_registered_fonts.push_back(this);
		};

		font_t::font_t(const std::string& type_name, const std::size_t height, const std::size_t weight, const std::uint8_t setup_flags, std::vector<std::uint8_t>& data)
			: m_type_name(type_name), m_height(height), m_weight(weight), m_setup_flags(setup_flags), m_data(&data), m_font_ptr(nullptr)
		{
			setup();
			m_registered_fonts.push_back(this);
		};

		font_t::~font_t()
		{
			unregister_font(this);
		}

		void font_t::setup()
		{
			if (m_can_setup_fonts && (m_reset_fonts || !m_font_ptr))
			{
				if (m_data && !m_data->empty())
					m_font_ptr = create_font_memory(m_type_name, m_height, m_weight, m_setup_flags, *m_data);
				else if (!m_type_name.empty())
					m_font_ptr = create_font(m_type_name, m_height, m_weight, m_setup_flags);
			}
		}

		template<typename T>
		void font_t::draw(math::point_t position, const color_t& color, const T& text, const std::uint8_t flags)
		{
			const math::point_t& text_size = get_text_size(text);
			if (flags & TEXTFLAG_LEFT)
				position.x -= text_size.x;
			else if (flags & TEXTFLAG_CENTERED_X)
				position.x -= text_size.x * 0.5f;

			if (flags & TEXTFLAG_BOTTOM)
				position.y -= text_size.y;
			else if (flags & TEXTFLAG_CENTERED_Y)
				position.y -= text_size.y * 0.5f;

			const color_t outline_color = { 5.f, 5.f, 5.f, color.a * 0.55f };

			if (flags & TEXTFLAG_OUTLINE || m_setup_flags & FONTFLAG_OUTLINE)
			{
				draw_text_<T>({ position.x - 1, position.y - 1 }, outline_color, text.data(), m_font_ptr, m_height);
				draw_text_<T>({ position.x - 1, position.y + 1 }, outline_color, text.data(), m_font_ptr, m_height);
				draw_text_<T>({ position.x + 1, position.y - 1 }, outline_color, text.data(), m_font_ptr, m_height);
				draw_text_<T>({ position.x + 1, position.y + 1 }, outline_color, text.data(), m_font_ptr, m_height);
			}

			if (flags & TEXTFLAG_DROPSHADOW || m_setup_flags & FONTFLAG_DROPSHADOW)
			{
				if (m_height >= 16)
				{
					draw_text_<T>({ position.x + 2, position.y }, outline_color, text.data(), m_font_ptr, m_height);
					draw_text_<T>({ position.x + 2, position.y + 2 }, outline_color, text.data(), m_font_ptr, m_height);
				}
				else
					draw_text_<T>({ position.x + 1, position.y + 1 }, outline_color, text.data(), m_font_ptr, m_height);
			}

			draw_text_<T>(position, color, text.data(), m_font_ptr, m_height);
		}

		template<typename T>
		void font_t::draw(math::point_t position, const int x, const color_t& color, const T& text, const std::uint8_t flags)
		{
			const math::point_t& text_size = get_text_size(text);
			if (flags & TEXTFLAG_LEFT)
				position.x -= text_size.x;
			else if (flags & TEXTFLAG_CENTERED_X)
				position.x -= text_size.x * 0.5f;

			if (flags & TEXTFLAG_BOTTOM)
				position.y -= text_size.y;
			else if (flags & TEXTFLAG_CENTERED_Y)
				position.y -= text_size.y * 0.5f;

			const color_t outline_color = { 5.f, 5.f, 5.f, color.a * 0.55f };

			if (flags & TEXTFLAG_OUTLINE || m_setup_flags & FONTFLAG_OUTLINE)
			{
				draw_text_cut_<T>({ position.x - 1, position.y - 1 }, x, outline_color, text.data(), m_font_ptr, m_height);
				draw_text_cut_<T>({ position.x - 1, position.y + 1 }, x, outline_color, text.data(), m_font_ptr, m_height);
				draw_text_cut_<T>({ position.x + 1, position.y - 1 }, x, outline_color, text.data(), m_font_ptr, m_height);
				draw_text_cut_<T>({ position.x + 1, position.y + 1 }, x, outline_color, text.data(), m_font_ptr, m_height);
			}

			if (flags & TEXTFLAG_DROPSHADOW || m_setup_flags & FONTFLAG_DROPSHADOW)
			{
				if (m_height >= 16)
				{
					draw_text_cut_<T>({ position.x + 2, position.y }, x, outline_color, text.data(), m_font_ptr, m_height);
					draw_text_cut_<T>({ position.x + 2, position.y + 2 }, x, outline_color, text.data(), m_font_ptr, m_height);
				}
				else
					draw_text_cut_<T>({ position.x + 1, position.y + 1 }, x, outline_color, text.data(), m_font_ptr, m_height);
			}

			draw_text_cut_<T>(position, x, color, text.data(), m_font_ptr, m_height);
		}

		template<typename T>
		math::point_t font_t::get_text_size(const T& text)
		{
			return ::rgui::drawing::get_text_size<T>(text, m_font_ptr, m_height);
		}

		template<typename T>
		math::point_t font_t::get_text_size(const T& text, const int x)
		{
			return ::rgui::drawing::get_text_size_cut_<T>(text, x, m_font_ptr, m_height);
		}

		template void font_t::draw<std::string>(math::point_t position, const color_t& color, const std::string& text, const std::uint8_t flags);
		template void font_t::draw<std::string_view>(math::point_t position, const color_t& color, const std::string_view& text, const std::uint8_t flags);
		template void font_t::draw<std::wstring>(math::point_t position, const color_t& color, const std::wstring& text, const std::uint8_t flags);
		template void font_t::draw<std::wstring_view>(math::point_t position, const color_t& color, const std::wstring_view& text, const std::uint8_t flags);

		template void font_t::draw<std::string>(math::point_t position, const int x, const color_t& color, const std::string& text, const std::uint8_t flags);
		template void font_t::draw<std::string_view>(math::point_t position, const int x, const color_t& color, const std::string_view& text, const std::uint8_t flags);
		template void font_t::draw<std::wstring>(math::point_t position, const int x, const color_t& color, const std::wstring& text, const std::uint8_t flags);
		template void font_t::draw<std::wstring_view>(math::point_t position, const int x, const color_t& color, const std::wstring_view& text, const std::uint8_t flags);

		template math::point_t font_t::get_text_size<std::string>(const std::string& text);
		template math::point_t font_t::get_text_size<std::string_view>(const std::string_view& text);
		template math::point_t font_t::get_text_size<std::wstring>(const std::wstring& text);
		template math::point_t font_t::get_text_size<std::wstring_view>(const std::wstring_view& text);

		template math::point_t font_t::get_text_size<std::string>(const std::string& text, const int x);
		template math::point_t font_t::get_text_size<std::string_view>(const std::string_view& text, const int x);
		template math::point_t font_t::get_text_size<std::wstring>(const std::wstring& text, const int x);
		template math::point_t font_t::get_text_size<std::wstring_view>(const std::wstring_view& text, const int x);
	}
}