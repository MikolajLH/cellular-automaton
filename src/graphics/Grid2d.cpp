#include "Grid2d.h"

Grid2d::Grid2d(size_t rows, size_t cols, std::span<const glm::vec2>cell_model) : 
	rows { rows },
	cols { cols },
	origin { 0.f, 0.f },
	scale { 1.f, 1.f },
	interspace { 0.f, 0.f },
	instances_colors(rows* cols, glm::vec3{}),
	instances_positions(rows* cols, glm::vec2{}),
	object{},
	shader(vert_shader, frag_shader) {
	DrawableObject::VertexLayout layout{};
	layout.push<float>("model_position", 2);
	object = DrawableObject(layout, to_byte_span(cell_model));


	layout.reset();
	layout.push<float>("instance_position", 2);
	object.add_instanced_vertex_buffer("instance_position", layout, to_byte_span(instances_positions));


	change_and_update_interspace(interspace);

	layout.reset();
	layout.push<float>("instance_color", 3);
	object.add_instanced_vertex_buffer("instance_color", layout, to_byte_span(instances_colors));
}

void Grid2d::change_and_update_interspace(const glm::vec2& new_interspace) {
	interspace = new_interspace;
	for (size_t r = 0; r < rows; ++r) {
		for (size_t c = 0; c < cols; ++c) {
			const auto fc = static_cast<float>(c);
			const auto fr = static_cast<float>(r);
			instances_positions[r * cols + c] = glm::vec2(fc * (interspace.x + scale.x), -fr * (interspace.y + scale.y));
		}
	}
	object.update_instanced_buffer("instance_position", to_byte_span(instances_positions));
}

void Grid2d::change_color(size_t r, size_t c, const glm::vec3& nc) {
	instances_colors[r * cols + c] = nc;
}

void Grid2d::update_colors() {
	object.update_instanced_buffer("instance_color", to_byte_span(instances_colors));
}

void Grid2d::draw(const glm::mat4& proj) {
	const auto model =
		glm::translate(glm::mat4(1.f), glm::vec3(origin, 0.f)) *
		glm::scale(glm::mat4(1.f), glm::vec3(scale, 1.f));

	shader.bind();
	shader.set_uniform_mat4f("u_model", model);
	shader.set_uniform_mat4f("u_proj", proj);

	object.draw_instanced(GL_TRIANGLE_FAN, rows * cols);

	shader.unbind();
}
