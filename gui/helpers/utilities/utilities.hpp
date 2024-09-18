#pragma once
#include <memory>

namespace rgui
{
	namespace utilities
	{
		template<typename T>
		inline const bool is(std::shared_ptr<void> variable)
		{
			return std::dynamic_pointer_cast<T>(variable);
		}

		template<typename T>
		inline const bool is(std::unique_ptr<void> variable)
		{
			return std::dynamic_pointer_cast<T>(variable);
		}
	}
}