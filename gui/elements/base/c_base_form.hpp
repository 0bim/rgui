#pragma once
#ifndef C_BASE_FORM_HPP
#define C_BASE_FORM_HPP
#include "c_base_parent.hpp"

namespace rgui
{
	namespace drawing
	{
		struct color_t;
	}

	namespace elements
	{
		class c_base_form : public c_base_parent, public c_base_draggable
		{
		public:
			c_base_form(const std::string& title, const math::point_t& position, const math::point_t & size);

		public:
			void draw() override;

			bool process_input(const input::input_capture_t& input, const float scroll) override;

			bool process_input(const char character) override;

			void update() override;

		public:
			inline void set_background_texture(void* const texture)
			{
				m_background_texture = texture;
			}

			inline void* get_texture() const
			{
				return m_background_texture;
			}

			inline void set_is_draggable(const bool flag)
			{
				m_is_draggable = flag;
			}
			
			inline bool get_is_draggable() const
			{
				return m_is_draggable;
			}

			inline void set_is_outlined(const bool flag)
			{
				m_is_outlined = flag;
			}

			inline bool get_is_outlined() const
			{
				return m_is_outlined;
			}

			inline void set_fill_background(const bool flag)
			{
				m_fill_background = flag;
			}

			inline bool get_fill_background() const
			{
				return m_fill_background;
			}

		protected:
			bool m_is_draggable;
			math::easing::animation_t m_open_animation, m_scroll_animation;
			bool m_reverse_scroll_animation;

			void* m_background_texture;

			bool m_is_outlined, m_fill_background;
		};
	}
}

#endif