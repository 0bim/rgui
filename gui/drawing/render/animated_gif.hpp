#pragma once
#include <d3d9.h>
#include <cstdint>
#include <vector>
#include <chrono>
#include <thread>
#include <future>
#include <unordered_map>
#include "../../helpers/math/point_t.hpp"

namespace rgui
{
	namespace drawing
	{
		struct animated_gif_info_t
		{
			animated_gif_info_t();

			animated_gif_info_t(const std::string& path);

			std::vector<std::uint8_t> read_file(const std::string_view path);

			void initialize_from_memory(std::uint8_t* data, const std::size_t size);

			std::uint8_t* m_data;
			int* m_delays;
			int m_frame_count; //z
			int m_comp;
			math::point_t m_dimensions;
			/*stride_bytes = x*comp; then use data + stride_bytes*y*i in i < z for loop*/
		};

		struct animated_gif_display_t
		{
			animated_gif_display_t()
				: m_gif_info(), m_frame_index(-1), m_frame_textures({ }), m_last_frame_timestamp(), m_begin_timestamp()//, m_texture_load_futures(0)
			{

			}

			animated_gif_display_t(const animated_gif_info_t& gif_info)
				: m_gif_info(gif_info), m_frame_index(-1), m_frame_textures(), m_last_frame_timestamp(), m_begin_timestamp()//, m_texture_load_futures(0)
			{
				m_frame_textures.reserve(gif_info.m_frame_count);
				create_gif_textures();
			}

			inline void create_gif_textures()
			{
				for (std::size_t i = 0; i < m_gif_info.m_frame_count; i++)
				{
					m_frame_textures.push_back(get_frame_texture(i));
					//m_texture_load_futures.push_back(std::async(std::launch::async, assign_texture, i));
				}
			}

			inline bool advance(const bool force_reset = false)
			{
				if (force_reset || get_has_ended() || m_frame_index < 0)
				{
					m_frame_index = 0;
					m_begin_timestamp = std::chrono::system_clock::now();
					m_last_frame_timestamp = m_begin_timestamp;
					return true;
				}
				else 
				{
					const int time_passed = get_time_since_last_frame();
					if (time_passed > get_frame_delay(m_frame_index))
					{
						m_frame_index++;
						m_last_frame_timestamp = std::chrono::system_clock::now();
						return true;
					}
				}

				return false;
			}

			inline bool get_has_ended() const
			{
				return m_frame_index >= m_gif_info.m_frame_count - 1;
				//return std::abs(duration_cast<std::chrono::milliseconds>((std::chrono::system_clock::now() - m_begin_timestamp)).count()) /*time since beginning*/ > get_gif_length_time();
			}

			inline int get_time_since_last_frame() const
			{
				return std::abs(duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_last_frame_timestamp).count());
			}

			inline int get_gif_length_time() const
			{
				int length = 0;
				for (std::size_t i = 0; i < m_gif_info.m_frame_count; i++)
				{
					length += m_gif_info.m_delays[i];
				}

				return length;
			}

			inline int get_frame_delay(const std::size_t frame_index) const
			{
				return m_gif_info.m_delays[frame_index];
			}

			std::vector<std::uint8_t> get_frame_data_png(const int frame_index);

			void* get_frame_texture(const int frame_index);

			void* get_frame_texture();

			animated_gif_info_t m_gif_info;
			int m_frame_index;
			std::chrono::system_clock::time_point m_last_frame_timestamp;
			std::chrono::system_clock::time_point m_begin_timestamp;

			std::vector<void*> m_frame_textures;

			//std::vector<std::future<void>> m_texture_load_futures;
		};
	}
}
