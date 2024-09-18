#pragma once
#include <array>

#include "vector.hpp"

namespace rgui
{
	namespace math
	{
		struct matrix4x4f
		{
			std::array<vector4d, 4> matrix = { };

			matrix4x4f() = default;

			matrix4x4f(const vector4d& row1, const vector4d& row2, const vector4d& row3, const vector4d& row4)
				: matrix{ row1, row2, row3, row4 } {}

			const vector4d& operator[](const size_t idx) const
			{
				return matrix[idx];
			}

			vector4d& operator[](const size_t idx)
			{
				return matrix[idx];
			}
		};
	}
}
