#pragma once
#ifndef C_TEXTURE_HPP
#define C_TEXTURE_HPP
#include <d3d9.h>
#include "../../helpers/math/point_t.hpp"

namespace rgui
{
	namespace drawing
	{
		class c_texture
		{
		public:
			c_texture()
				: m_texture_ptr(nullptr), m_size({ 0, 0 })
			{

			}

			c_texture(IUnknown* texture_d3d, const math::point_t& size, const float scale = 1.f)
				: m_texture_ptr(texture_d3d), m_size(size), m_previous_size(size)
			{

			}

			c_texture(void* texture_d3d, const math::point_t& size, const float scale = 1.f)
				: m_texture_ptr(static_cast<IUnknown*>(texture_d3d)), m_size(size), m_previous_size(size)
			{

			}

			~c_texture()
			{
				if (m_texture_ptr)
				{
					m_texture_ptr->Release();
					m_texture_ptr = nullptr;
				}

				m_size = { };
			}

		public:
			inline bool is_valid() const
			{
				return this && m_texture_ptr;
			}

			inline void* get_texture_ptr() const
			{
				return m_texture_ptr;
			}

			inline void set_size(const math::point_t& size)
			{
				m_size = size;
			}

			inline math::point_t get_size(const float scale = 1.f) const
			{
				return m_size * scale;
			}

			inline void scale_size(const float scale)
			{
				m_previous_size = m_size;
				m_size *= scale;
			}

			inline void restore_size()
			{
				m_size = m_previous_size;
			}

		private:
			IUnknown* m_texture_ptr;
			math::point_t m_size;
			math::point_t m_previous_size;
		};
	}
}
#endif