#pragma once
#ifndef C_ELEMENT_HPP
#define C_ELEMENT_HPP
#include <cstdint>
#include "c_stylesheet.hpp"
#include "../../helpers/math/point_t.hpp"
#include "../../helpers/input/input.hpp"
#include "../../helpers/math/animation.hpp"
#include "../../drawing/render/render.hpp"

namespace rgui
{
	namespace elements
	{
		enum class E_ELEMENT_FLAG : std::uint8_t
		{
			NONE = (1 << 0), 
			NO_TARGET = (1 << 1)
		};

		class c_base_parent;

		class c_base_element : public std::enable_shared_from_this<c_base_element>
		{
		public:
			friend class c_base_parent;

			c_base_element();

			c_base_element(const std::string& title, const math::point_t& position, const math::point_t& size);

		public:
			virtual void draw() = 0; //or paint idk what to call it :/

			virtual bool process_input(const input::input_capture_t& input, const float scroll) = 0;

			virtual bool process_input(const char character);

			virtual void update() = 0;

			virtual bool get_is_visible() const;

		public:
			bool get_is_target_element();

			shapes::rect_t get_geometry_area_relative() const;

			std::string get_title_formatted();

		public:
			inline virtual void set_title(const std::string_view title)
			{
				m_title = title;
			}

			inline std::string_view get_title() const
			{
				return m_title;
			}

			inline virtual void set_position(const math::point_t& position)
			{
				m_position = position;
			}

			inline virtual math::point_t get_position() const
			{
				return m_position.scale_ascpect_ratio(m_scale);
			}

			inline virtual void set_size(const math::point_t& size)
			{
				m_size = size;
			}

			inline math::point_t get_size(const bool unscaled = false) const
			{
				return unscaled ? m_size : m_size.scale_ascpect_ratio(m_scale);
			}

			inline shapes::rect_t get_geometry_area() const
			{
				return { get_position(), get_size() };
			}

			inline virtual shapes::rect_t get_complete_area() const
			{
				return { };
			}

			inline virtual void set_parent(std::shared_ptr<c_base_parent> parent)
			{
				m_parent = parent;
			}

			inline std::shared_ptr<c_base_parent> get_parent() const
			{
				return m_parent;
			}

			inline bool get_flag(const E_ELEMENT_FLAG flag) const
			{
				return m_flags & static_cast<std::uint8_t>(flag);
			}

			inline virtual void on_retarget()
			{
				//idk what to do here atm
			}

			inline void set_is_visible(const bool visible)
			{
				m_is_visible = visible;
			}

			inline c_stylesheet::style_t* get_style() const
			{
				return m_style;
			}

			inline virtual void on_move(const math::point_t& delta)
			{
				m_position += delta;
			}

			inline virtual void set_port_area(const math::point_t& area)
			{
				m_port_area = area;
			}

			inline virtual math::point_t get_port_area()
			{
				return m_port_area.scale_ascpect_ratio(m_scale);
			}

			inline void set_rounding(const int rounding)
			{
				m_rounding = rounding;
			}

			inline int get_rounding() const
			{
				return m_rounding;
			}

			inline void set_is_input_enabled(const bool flag)
			{
				m_enable_input = flag;
			}

			inline bool get_is_input_enabled() const
			{
				return m_enable_input;
			}

			inline void set_ignore_scroll(const bool flag)
			{
				m_ignore_scroll = flag;
			}

			inline bool get_ignore_scroll() const
			{
				return m_ignore_scroll;
			}

			inline virtual std::shared_ptr<c_base_element> find_child_traverse(const std::string& title)
			{
				return m_title == title ? shared_from_this() : nullptr;
			}

		protected:
			inline void push_flag(const E_ELEMENT_FLAG flag)
			{
				m_flags |= static_cast<std::uint8_t>(flag);
			}

			inline void pop_flag(const E_ELEMENT_FLAG flag)
			{
				m_flags &= ~static_cast<std::uint8_t>(flag);
			}

		public:
			inline static float m_scale = 1.f;

		protected:
			std::string m_title;

			math::point_t m_position;
			math::point_t m_size;

			std::uint8_t m_flags;

			bool m_is_visible;

			std::shared_ptr<c_base_parent> m_parent;

			math::point_t m_port_area;

			int m_rounding;

			c_stylesheet::style_t* m_style;

			bool m_enable_input;

			bool m_ignore_scroll;

			bool m_destruct_post_use;
		};
	}
}
#endif