#pragma once
#ifndef C_POPUP_HPP
#define C_POPUP_HPP
#include "base/c_base_parent.hpp"
#include "base/c_base_openable.hpp"
#include "base/c_base_animated.hpp"

namespace rgui
{
	namespace elements
	{
		class c_button; //forward declaration

		class c_popup : public c_base_parent, public c_base_openable, public c_base_animated
		{
		public:
			c_popup(const std::string& title, const math::point_t& size);

		public:
			//declare as empty functions because they must be overridden
			virtual void draw() override;

			virtual bool process_input(const input::input_capture_t& input, const float scroll) override;

			virtual void update();

		public:
			void set_parent(std::shared_ptr<c_base_parent> parent) override final;
		};
	}
}

#endif