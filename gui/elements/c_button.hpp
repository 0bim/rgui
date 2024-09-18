#pragma once
#ifndef C_BUTTON_HPP
#define C_BUTTON_HPP
#include "base/c_base_element.hpp"
#include "base/c_base_animated.hpp"
#include "base/c_base_icon.hpp"
#include "base/c_base_textured.hpp"
#include "base/c_base_callback.hpp"

namespace rgui
{
	namespace elements
	{
		class c_button : public c_base_element, public c_base_animated, public c_base_icon, public c_base_textured, public c_base_callback
		{
		public:
			c_button(const std::string& title, const math::point_t& position, const math::point_t& size = { 10, 10 });

		public:
			void draw() override;

			bool process_input(const input::input_capture_t& input, const float scroll) override;

			void update() override;

		public:
			inline void set_animate(const bool animate)
			{
				m_animate = animate;
			}

			inline void set_is_outlined(const bool flag)
			{
				m_outlined = flag;
			}

			inline bool get_is_outlined() const
			{
				return m_outlined;
			}

			inline void set_dynamic_size(const bool flag)
			{
				m_dynamic_size = flag;
				if (m_dynamic_size)
					set_size({ m_style->get_font(E_STYLE_FONT::MEDIUM)->get_text_size(get_title_formatted()).x + 5, std::max(m_style->get_font(E_STYLE_FONT::MEDIUM)->get_text_size(get_title_formatted()).y + 5, m_size.y) });
			}

			inline bool get_dynamic_size() const
			{
				return m_dynamic_size;
			}

			inline void set_is_background_dependant(const bool flag)
			{
				m_is_background_dependant = flag;
			}

			inline bool get_is_background_dependant() const
			{
				return m_is_background_dependant;
			}

			inline void set_has_plus(const bool flag)
			{
				m_has_plus = flag;
			}

			inline bool get_has_plus() const
			{
				return m_has_plus;
			}

		private:
			bool m_animate;
			bool m_outlined;
			bool m_is_background_dependant;
			bool m_dynamic_size;
			bool m_has_plus;
		};
	}
}

#endif