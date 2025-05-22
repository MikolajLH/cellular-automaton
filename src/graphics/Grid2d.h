#include <span>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "DrawableObject.h"
#include "ShaderProgram.h"

class Grid2d {
	/*
	This class describes a 2 dimensional grid of objects of the same shape,
	It uses the openGL instaces attribute to render many shapes effectivly,
	Each shape can have distinct color that can be changed in an update loop with the change_color function
	In order to the change of colors to be visible the instances' colors vector has to be updated on the GPU side with the update_colors function
	*/
public:
	Grid2d(size_t rows, size_t cols, std::span<const glm::vec2>cell_model);

	void draw(const glm::mat4& proj);
	void change_and_update_interspace(const glm::vec2& new_interpsace);
	void change_color(size_t r, size_t c,const glm::vec3& new_color);
	void update_colors();

	glm::vec2 origin;
	glm::vec2 scale;
	glm::vec2 interspace;
	std::vector<glm::vec3>instances_colors;
	//private:
	size_t rows;
	size_t cols;
	DrawableObject object;
	ShaderProgram shader;
	std::vector<glm::vec2>instances_positions;

	static constexpr std::string_view vert_shader =
		"#version 330 core\n"
		"layout(location = 0) in vec2 a_position;\n"
		"\n"
		"layout(location = 1) in vec2 ia_position;\n"
		"layout(location = 2) in vec3 ia_color;\n"
		"\n"
		"uniform mat4 u_model;\n"
		"uniform mat4 u_proj;\n"
		"\n"
		"out vec3 f_color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	mat4 mat;\n"
		"	mat[0] = vec4(1.0, 0.0, 0.0, 0.0);\n"
		"	mat[1] = vec4(0.0, 1.0, 0.0, 0.0);\n"
		"	mat[2] = vec4(0.0, 0.0, 1.0, 0.0);\n"
		"	mat[3] = vec4(ia_position, 0.0, 1.0);\n"
		"\n"
		"	gl_Position = u_proj * mat * u_model * vec4(a_position, 0.0, 1.0);\n"
		"	f_color = ia_color;\n"
		"}\n";


	static constexpr std::string_view frag_shader =
		"#version 330 core\n"
		"\n"
		"out vec4 color;\n"
		"\n"
		"in vec3 f_color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	color = vec4(f_color, 1.0);\n"
		"}\n";
};