#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "vector.hpp"
#include "../shapes/shapes.hpp"
#include "../../drawing/color/color.hpp"

namespace rgui
{
	namespace math
	{
		struct vertex_t
		{
			vertex_t() : m_position({ 0.f, 0.f, 0.f, 0.f }), m_color(drawing::color_t::white().to_hex()), m_texture(vector2d::zero()) { };

			vertex_t(const math::vector2d& position, const D3DCOLOR color, const vector2d& texture = vector2d::zero()) : m_position({ position.x, position.y, 1.f, 1.f }), m_color(color), m_texture(texture) { }

			vertex_t(const math::vector2d& position, const drawing::color_t& color, const vector2d& texture = vector2d::zero()) : m_position({ position.x, position.y, 1.f, 1.f }), m_color(color.to_hex()), m_texture(texture) { }

			D3DXVECTOR4 m_position;
			D3DCOLOR m_color;
			vector2d m_texture;
		};
	}
}