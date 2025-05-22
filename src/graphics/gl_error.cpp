#include "gl_error.h"


void log_gl_error(std::string_view fn_name) {
	bool err = false;
	while (GLenum error = glGetError())
	{
		if (not err) {
			std::println("Errors after {}:", fn_name);
		}
		std::println("[OpenGL Error]: {}", error);
	}
}