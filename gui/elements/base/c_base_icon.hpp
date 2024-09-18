#pragma once
#ifndef C_BASE_ICON
#define C_BASE_ICON

namespace rgui
{
	namespace elements
	{
		class c_base_icon
		{
		public:
			c_base_icon(const char icon = '\0') : m_icon(icon)
			{
			}

		public:
			inline void set_icon(const char icon)
			{
				m_icon = icon;
			}

			inline char get_icon() const
			{
				return m_icon;
			}

			inline void set_icon_font(drawing::font_t* font)
			{
				m_icon_font = font;
            }

			inline drawing::font_t* get_icon_font()
			{
				return m_icon_font;
			}

		protected:
			char m_icon;

			drawing::font_t* m_icon_font;
		};
	}
}

#endif