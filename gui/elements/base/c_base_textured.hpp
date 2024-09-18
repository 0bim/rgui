#pragma once
#ifndef C_BASE_TEXTURED_HPP
#define C_BASE_TEXTURED_HPP
#include "../../drawing/render/render.hpp"

namespace rgui
{
	namespace elements
	{
		class c_base_textured
		{
		public:
			c_base_textured(drawing::c_texture* texture = nullptr)
				: m_texture(texture)
			{

			}

		public:
			inline void set_texture(drawing::c_texture* texture)
			{
				m_texture = texture;
			}

			inline drawing::c_texture* get_texture() const
			{
				return m_texture;
			}

		protected:
			drawing::c_texture* m_texture;
		};
	}
}

#endif