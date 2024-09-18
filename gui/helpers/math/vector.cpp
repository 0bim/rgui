#include "vector.hpp"
#include "point_t.hpp"

namespace rgui
{
	namespace math
	{
		vector2d::vector2d(const point_t& point)
			: x(point.x), y(point.y)
		{

		}
	}
}