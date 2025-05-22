#include "DrawableObject.h"

DrawableObject& DrawableObject::operator=(DrawableObject&& other) noexcept {
	m_vertex_array_object = std::exchange(other.m_vertex_array_object, 0u);

	m_model_vertex_buffer_object = std::exchange(other.m_model_vertex_buffer_object, 0u);
	m_model_vertex_count = std::exchange(other.m_model_vertex_count, 0u);

	next_attrib_index = std::exchange(other.next_attrib_index, 0u);

	m_instance_vertex_buffer_objects_map = std::move(other.m_instance_vertex_buffer_objects_map);

	attribs_info = std::move(other.attribs_info);

	return *this;
}


DrawableObject::DrawableObject(DrawableObject&& other) noexcept {
	(*this) = std::move(other);
}

DrawableObject::~DrawableObject() {
	if (m_vertex_array_object)
		GLLOGERROR(glDeleteVertexArrays(1, &m_vertex_array_object));
	if (m_model_vertex_buffer_object)
		GLLOGERROR(glDeleteBuffers(1, &m_model_vertex_buffer_object));

	for (auto& [name, triple] : m_instance_vertex_buffer_objects_map) {
		auto& [ivbo, vert_count, byte_count] = triple;
		GLLOGERROR(glDeleteBuffers(1, &ivbo));
	}

}


DrawableObject::DrawableObject(const VertexLayout& layout, std::span<const std::byte> model_vertex_bytes) {
	assert(layout.get_width() != 0u);
	assert(model_vertex_bytes.size() != 0u);

	m_model_vertex_count = model_vertex_bytes.size() / layout.get_width();

	// generate vertex array and bind it 
	GLLOGERROR(glGenVertexArrays(1, &m_vertex_array_object));
	GLLOGERROR(glBindVertexArray(m_vertex_array_object));
	//

	// generate vertex buffer, bind it and copy vertices to opengl 
	GLLOGERROR(glGenBuffers(1, &m_model_vertex_buffer_object));
	GLLOGERROR(glBindBuffer(GL_ARRAY_BUFFER, m_model_vertex_buffer_object));
	GLLOGERROR(glBufferData(GL_ARRAY_BUFFER, model_vertex_bytes.size(), model_vertex_bytes.data(), GL_STATIC_DRAW));
	//

	//unbind vertex array, leave buffers bounded because they were bounded after vao so they are linked to vao state, not global state
	GLLOGERROR(glBindBuffer(GL_ARRAY_BUFFER, 0u));
	GLLOGERROR(glBindVertexArray(0));

	link_layout(layout, m_model_vertex_buffer_object, false);
}

void DrawableObject::link_layout(const VertexLayout& layout, GLuint vbo, bool is_instanced) {

	GLLOGERROR(glBindVertexArray(m_vertex_array_object));
	GLLOGERROR(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	for (auto& [layout_index, num_components, type, offset] : layout.m_attribs)
	{
		const auto curr_attrib_index = static_cast<GLuint>(layout_index + next_attrib_index);

		GLLOGERROR(glEnableVertexAttribArray(curr_attrib_index));
		GLLOGERROR(glVertexAttribPointer(curr_attrib_index, num_components, type, GL_FALSE, static_cast<GLsizei>(layout.current_stride), (void*)offset));

		glVertexAttribDivisor(curr_attrib_index, static_cast<GLuint>(is_instanced));

		attribs_info.emplace_back(layout.m_attribs_names[layout_index], curr_attrib_index, layout_index, num_components, type, is_instanced, layout.current_stride, offset);
	}
	next_attrib_index += layout.m_attribs.size();


	GLLOGERROR(glBindBuffer(GL_ARRAY_BUFFER, 0u));
	GLLOGERROR(glBindVertexArray(0u));
}


void DrawableObject::add_instanced_vertex_buffer(std::string_view name, const VertexLayout& layout, std::span<const std::byte> vertex_bytes) {
	assert(layout.get_width() != 0u);
	assert(vertex_bytes.size() != 0u);
	assert(not m_instance_vertex_buffer_objects_map.contains(name));


	const size_t vertex_count = vertex_bytes.size() / layout.get_width();

	GLuint new_vbo{};
	GLLOGERROR(glGenBuffers(1, &new_vbo));
	GLLOGERROR(glBindBuffer(GL_ARRAY_BUFFER, new_vbo));
	GLLOGERROR(glBufferData(GL_ARRAY_BUFFER, vertex_bytes.size(), vertex_bytes.data(), GL_STATIC_DRAW));
	GLLOGERROR(glBindBuffer(GL_ARRAY_BUFFER, 0u));

	link_layout(layout, new_vbo, true);
	m_instance_vertex_buffer_objects_map.emplace(name, std::make_tuple(new_vbo, vertex_count, vertex_bytes.size()));
}


void DrawableObject::update_instanced_buffer(std::string_view name, std::span<const std::byte> new_buffer_data) {
	auto search_it = m_instance_vertex_buffer_objects_map.find(name);
	assert(search_it != m_instance_vertex_buffer_objects_map.end());
	auto [ivbo, vert_count, byte_count] = search_it->second;



	GLLOGERROR(glBindBuffer(GL_ARRAY_BUFFER, ivbo));
	void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, new_buffer_data.size(), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	std::memcpy(ptr, new_buffer_data.data(), new_buffer_data.size());
	glUnmapBuffer(GL_ARRAY_BUFFER);
	GLLOGERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	const size_t width = byte_count / vert_count;
	const size_t new_byte_count = new_buffer_data.size();
	const size_t new_vert_count = new_byte_count / width;

	search_it->second = std::make_tuple(ivbo, new_vert_count, new_byte_count);
}

void DrawableObject::assert_constructed()const {
	assert(next_attrib_index);
	assert(m_vertex_array_object);
	assert(m_model_vertex_buffer_object);
	assert(m_model_vertex_count);
	assert(attribs_info.size());
}

void DrawableObject::show_layout_info() {
	std::println("Layout info:");
	for (const auto& [name, out_index, in_index, n_comp, type, is_instanced, stride, offset] : attribs_info) {
		using namespace std::string_view_literals;
		const auto type_name =
			type == GL_FLOAT ? "GL_FLOAT"sv :
			type == GL_INT ? "GL_INT"sv :
			"Unknown";

		std::println("[{}] {} ({}): {} * {} | [{}] --- {} | {}",
			out_index, name, (is_instanced ? "instanced"sv : "vertex"sv),
			n_comp, type_name, in_index, stride, offset);
	}
}

void DrawableObject::draw(GLenum mode) {
	assert_constructed();
	GLLOGERROR(glBindVertexArray(m_vertex_array_object));
	GLLOGERROR(glDrawArrays(mode, 0, static_cast<GLsizei>(m_model_vertex_count)));
	GLLOGERROR(glBindVertexArray(0u));
}

void DrawableObject::draw_instanced(GLenum mode, size_t count) {
	assert_constructed();
	GLLOGERROR(glBindVertexArray(m_vertex_array_object));
	GLLOGERROR(glDrawArraysInstanced(mode, 0, static_cast<GLsizei>(m_model_vertex_count), static_cast<GLsizei>(count)));
	GLLOGERROR(glBindVertexArray(0u));
}
