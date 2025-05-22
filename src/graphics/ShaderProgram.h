#pragma once
#include <string_view>
#include <string>
#include <map>
#include <print>
#include <cassert>
#include <fstream>


#include "gl_error.h"
#include <glm/glm.hpp>


std::string read_file(std::string_view filepath);


class ShaderProgram
{
public:
	ShaderProgram(const ShaderProgram&) = delete;
	ShaderProgram& operator=(const ShaderProgram&) = delete;

	ShaderProgram() = default;
	ShaderProgram(ShaderProgram&&) noexcept;
	void operator=(ShaderProgram&&) noexcept;

	ShaderProgram(std::string_view vert_shader_src, std::string_view frag_shader_src);
	~ShaderProgram() noexcept;

	void bind() const noexcept;
	void unbind() const noexcept;

	void set_uniform_1i(std::string_view name, int value)const noexcept;
	void set_uniform_1f(std::string_view name, float value)const noexcept;
	void set_uniform_2f(std::string_view name, float v0, float v1)const noexcept;
	void set_uniform_3f(std::string_view name, float v0, float v1, float v2)const noexcept;
	void set_uniform_4f(std::string_view name, float v0, float v1, float v2, float v3)const noexcept;

	void set_uniform_mat4f(std::string_view name, const glm::mat4& mat);


	GLuint get_uniform_location(std::string_view name)const noexcept;

private:
	GLuint m_program_id{};
	mutable std::map<std::string, GLint, std::less<>> m_uniform_location_cache{};

	static GLuint compile_shader(GLuint type, std::string_view source);
};
