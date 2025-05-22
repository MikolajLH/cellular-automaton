#include "ShaderProgram.h"
#include <print>

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
	:
	m_program_id{ std::exchange(other.m_program_id, 0u) },
	m_uniform_location_cache{ std::move(other.m_uniform_location_cache) }
{
}


void ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
	m_program_id = std::exchange(other.m_program_id, 0u);
	m_uniform_location_cache = std::move(other.m_uniform_location_cache);
}


ShaderProgram::ShaderProgram(std::string_view vert_shader_src, std::string_view frag_shader_src) {

	GLLOGERROR(m_program_id = glCreateProgram());
	assert(m_program_id != 0);

	const auto vert_shader_id = compile_shader(GL_VERTEX_SHADER, vert_shader_src);
	const auto frag_shader_id = compile_shader(GL_FRAGMENT_SHADER, frag_shader_src);
	assert(vert_shader_id != 0 and frag_shader_id != 0);

	GLLOGERROR(glAttachShader(m_program_id, vert_shader_id));
	GLLOGERROR(glAttachShader(m_program_id, frag_shader_id));

	GLint result{};

	GLLOGERROR(glLinkProgram(m_program_id));
	GLLOGERROR(glGetProgramiv(m_program_id, GL_LINK_STATUS, &result));
	if (result == GL_FALSE) {
		std::println("Failed to link shader program");
		assert(false);
	}

	GLLOGERROR(glValidateProgram(m_program_id));
	GLLOGERROR(glGetProgramiv(m_program_id, GL_VALIDATE_STATUS, &result));
	if (result == GL_FALSE) {
		std::println("Failed to validate shader program");
		assert(false);
	}

	GLLOGERROR(glDeleteShader(vert_shader_id));
	GLLOGERROR(glDeleteShader(frag_shader_id));
}


void ShaderProgram::bind() const noexcept {
	if (m_program_id)
		GLLOGERROR(glUseProgram(m_program_id));
}

void ShaderProgram::unbind() const noexcept {
	if (m_program_id)
		GLLOGERROR(glUseProgram(0));
}

ShaderProgram::~ShaderProgram() noexcept {
	if (m_program_id)
		GLLOGERROR(glDeleteProgram(m_program_id));
}


GLuint ShaderProgram::compile_shader(GLuint type, std::string_view source) {
	GLLOGERROR(const GLuint sid = glCreateShader(type));
	const char* src = source.data();
	GLLOGERROR(glShaderSource(sid, 1, &src, nullptr));
	GLLOGERROR(glCompileShader(sid));

	GLint result{};
	GLLOGERROR(glGetShaderiv(sid, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE) {
		GLint length{};
		GLLOGERROR(glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &length));

		char buffer[1024];
		if (length >= 1024)
			assert(false);

		GLLOGERROR(glGetShaderInfoLog(sid, length, &length, buffer));

		std::println("Failed to compile shader!\n{}", buffer);
		GLLOGERROR(glDeleteShader(sid));
		return 0;
	}

	return sid;
}


GLuint ShaderProgram::get_uniform_location(std::string_view name)const noexcept {
	if (not m_uniform_location_cache.contains(name)) {
		GLLOGERROR(const auto in_shader_location = glGetUniformLocation(m_program_id, name.data()));
		assert(in_shader_location != -1);
		m_uniform_location_cache.emplace(std::string(name), in_shader_location);
	}
	return m_uniform_location_cache.find(name)->second;
}


void ShaderProgram::set_uniform_1i(std::string_view name, int value) const noexcept {
	GLLOGERROR(glUniform1i(get_uniform_location(name), value));
}

void ShaderProgram::set_uniform_1f(std::string_view name, float value) const noexcept {
	GLLOGERROR(glUniform1f(get_uniform_location(name), value));
}

void ShaderProgram::set_uniform_2f(std::string_view name, float v0, float v1)const noexcept {
	GLLOGERROR(glUniform2f(get_uniform_location(name), v0, v1));
}

void ShaderProgram::set_uniform_3f(std::string_view name, float v0, float v1, float v2)const noexcept {
	GLLOGERROR(glUniform3f(get_uniform_location(name), v0, v1, v2));
}

void ShaderProgram::set_uniform_4f(std::string_view name, float v0, float v1, float v2, float v3) const noexcept {
	GLLOGERROR(glUniform4f(get_uniform_location(name), v0, v1, v2, v3));
}

void ShaderProgram::set_uniform_mat4f(std::string_view name, const glm::mat4& mat) {
	GLLOGERROR(glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, &mat[0][0]));
}

std::string read_file(std::string_view filepath) {
	std::ifstream file(filepath.data());
	if (not file.is_open())
		throw std::runtime_error("File does not exist");
	const auto fbeg = file.tellg();
	file.seekg(0, std::ios::end);
	const auto fend = file.tellg();
	const auto fsize = (fend - fbeg);
	/*
	if (fsize == -1)
		throw std::runtime_error("Error with checking file size");*/
	file.seekg(0, std::ios::beg);

	std::string res(fsize, '\0');
	file.read(res.data(), fsize);
	return res;
}