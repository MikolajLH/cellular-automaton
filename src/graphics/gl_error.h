#pragma once
#include <print>
#include <string_view>
#include <span>
#include <GL/glew.h>

void log_gl_error(std::string_view expr_name);
#define GLLOGERROR(expr) expr;log_gl_error(#expr);