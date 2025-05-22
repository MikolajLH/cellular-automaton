#pragma once
#include <vector>
#include <tuple>
#include <span>
#include <string>
#include <print>
#include <string_view>
#include <map>
#include <cassert>
#include <ranges>


#include "gl_error.h"


template<std::ranges::contiguous_range Range>
auto to_byte_span(const Range& range) {
	return std::as_bytes(std::span(std::ranges::cbegin(range), std::ranges::cend(range)));
}


class DrawableObject
{
public:

	class VertexLayout
	{
	public:
		friend class DrawableObject;

		template<typename T>
		void push(std::string_view attrib_name, GLuint num_of_components_per_attrib) {
			if constexpr (std::same_as<T, float>) {
				const auto layout_index = m_attribs.size();
				m_attribs.emplace_back(
					static_cast<GLuint>(layout_index),
					num_of_components_per_attrib,
					GL_FLOAT, current_stride);
				m_attribs_names.emplace_back(attrib_name);
				current_stride += num_of_components_per_attrib * sizeof(float);
			}
		}
		size_t get_width()const noexcept { return current_stride; }

		void reset() noexcept {
			m_attribs.clear();
			m_attribs_names.clear();
			current_stride = 0u;
		}
	private:
		std::vector<std::tuple<GLuint, GLuint, GLenum, size_t>>m_attribs{};
		std::vector<std::string> m_attribs_names{};

		size_t current_stride{};
	};

private:
	GLuint m_vertex_array_object{};

	GLuint m_model_vertex_buffer_object{};
	size_t m_model_vertex_count{};

	std::map<std::string, std::tuple<GLuint, size_t, size_t>, std::less<>>m_instance_vertex_buffer_objects_map{};
public:
	DrawableObject() noexcept = default;

	DrawableObject(const DrawableObject&) = delete;
	DrawableObject& operator=(const DrawableObject&) = delete;

	DrawableObject(DrawableObject&&) noexcept;
	DrawableObject& operator=(DrawableObject&&) noexcept;

	~DrawableObject();
	//------------------------------

	DrawableObject(const VertexLayout& layout, std::span<const std::byte> model_vertex_bytes);

	void add_instanced_vertex_buffer(std::string_view name, const VertexLayout& layout, std::span<const std::byte> vertex_bytes);
	void update_instanced_buffer(std::string_view name, std::span<const std::byte> new_buffer_data);

	void draw(GLenum mode);
	void draw_instanced(GLenum mode, size_t instance_count);

	void show_layout_info();
	void assert_constructed() const;
private:
	//--------------------      name  , out_i , in_i  , n_comp, type  , inst, stride, offset  ------------
	std::vector<std::tuple<std::string, GLuint, GLuint, GLuint, GLenum, bool, size_t, size_t>>attribs_info{};
	void link_layout(const VertexLayout& layout, GLuint vbo, bool is_instanced);
	size_t next_attrib_index{};
};