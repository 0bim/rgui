#include <fstream>
#include <fstream>
#include "animated_gif.hpp"
#include "../../helpers/utilities/stb_image.hpp"
#include "../../helpers/utilities/stb_image_write.hpp"
#include "../render/render.hpp"

namespace rgui
{
	namespace drawing
	{
		animated_gif_info_t::animated_gif_info_t()
		{

		}

		animated_gif_info_t::animated_gif_info_t(const std::string& path)
		{
			std::size_t size = 0;
			auto file_data = read_file(path);
			initialize_from_memory(file_data.data(), file_data.size());
		}

		std::vector<std::uint8_t> animated_gif_info_t::read_file(const std::string_view path)
		{
			std::ifstream input(path.data(), std::ios_base::binary);

			std::vector<std::uint8_t> buffer((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>()));

			input.close();

			return buffer;
		}

		void animated_gif_info_t::initialize_from_memory(std::uint8_t* file_data, const std::size_t size)
		{
			stbi_flip_vertically_on_write(false);
			stbi_set_flip_vertically_on_load(false);
			stbi_set_unpremultiply_on_load(false);
			stbi_set_flip_vertically_on_load_thread(false);
			m_data = stbi_load_gif_from_memory(file_data, size, &m_delays, &m_dimensions.x, &m_dimensions.y, &m_frame_count, &m_comp, 4);
		}

		std::vector<std::uint8_t> animated_gif_display_t::get_frame_data_png(const int frame_index)
		{
			const math::point_t& dimensions = m_gif_info.m_dimensions;
			const int stride_bytes = dimensions.x * m_gif_info.m_comp;
			//stbi_write_png(ch, x, y, comp, data + stride_bytes*y*i, 0);
			stbi_flip_vertically_on_write(false);
			stbi_set_flip_vertically_on_load(false);
			stbi_set_unpremultiply_on_load(false);
			stbi_set_flip_vertically_on_load_thread(false);
			int size = 0;
			std::uint8_t* png_data = stbi_write_png_to_mem(m_gif_info.m_data + stride_bytes * dimensions.y * frame_index, stride_bytes, dimensions.x, dimensions.y, m_gif_info.m_comp, &size);
			std::vector<std::uint8_t> ret = { png_data, png_data + size };
			stbi_image_free(png_data);

			return ret;
		}

		void* animated_gif_display_t::get_frame_texture(const int frame_index)
		{
			if (m_frame_textures.size() <= frame_index || !m_frame_textures.at(frame_index))
			{
				const auto& frame_data = get_frame_data_png(frame_index);
				auto texture = create_texture_file_memory(std::string(frame_data.begin(), frame_data.end()));
				return texture;

			}

			return m_frame_textures.at(frame_index);
		}

		void* animated_gif_display_t::get_frame_texture()
		{
			return get_frame_texture(m_frame_index);
		}
	}
}