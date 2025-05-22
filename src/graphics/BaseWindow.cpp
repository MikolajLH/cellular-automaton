#include "BaseWindow.h"


namespace wnd
{
	BaseWindow::BaseWindow(std::uint16_t w, std::uint16_t h, std::string_view title)
		: width{ w }, height{ h }, p_wnd{ nullptr } {

		/*glfw init*/ {
				glfwSetErrorCallback(
					[](int error, const char* description) {
						std::println("GLFW Error [{}]: {}", error, description);
					});

				const auto glfw_init_err = glfwInit();
				assert(glfw_init_err);

				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			}//

		p_wnd = glfwCreateWindow(
			static_cast<int>(width),
			static_cast<int>(height),
			title.data(),
			nullptr, nullptr);
		assert(p_wnd);

		glfwMakeContextCurrent(p_wnd);

		glfwSetWindowUserPointer(p_wnd, this);

		glfwSetWindowSizeCallback(
			p_wnd,
			[]([[maybe_unused]] GLFWwindow* pwnd, int w, int h) {
				glViewport(0, 0, w, h);
				auto pthis = reinterpret_cast<BaseWindow*>(glfwGetWindowUserPointer(pwnd));
				pthis->set_window_size(static_cast<std::uint16_t>(w), static_cast<std::uint16_t>(h));
			});

		glfwSwapInterval(1);

		/*GLEW init*/ {
			const auto glew_init_err = glewInit();
			if (glew_init_err != GLEW_OK) {

				const auto err_msg =
					std::basic_string_view<unsigned char>(glewGetErrorString(glew_init_err)) |
					std::views::transform([](unsigned char c) { return static_cast<char>(c); }) |
					std::ranges::to<std::string>();

				std::println("{}", err_msg);
				assert(false);
			}

			glDebugMessageCallback(
				[](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void*) {
					std::println("GL Debug Message:\nsource: {} type: {} id: {} severity: {}\n message: {}",
						source, type, id, severity, std::string_view(message, length));
				}, nullptr);
		}//

		/*imgui init*/ {
			//imgui
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

			// Setup Dear ImGui style
			ImGui::StyleColorsDark();
			//ImGui::StyleColorsLight();

			constexpr const char* glsl_version = "#version 130";

			// Setup Platform/Renderer backends
			ImGui_ImplGlfw_InitForOpenGL(this->p_wnd, true);
			ImGui_ImplOpenGL3_Init(glsl_version);
		}//
	}

	BaseWindow::~BaseWindow() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();


		glfwTerminate();
	}


	float BaseWindow::get_width()const noexcept { return static_cast<float>(width); }
	float BaseWindow::get_height()const noexcept { return static_cast<float>(height); }

	void BaseWindow::set_window_size(std::uint16_t w, std::uint16_t h) noexcept {
		width = w;
		height = h;
	}


	void BaseWindow::update([[maybe_unused]] float dt) {

		clear_screen(1.0f, 0.f, 0.5f);
		static bool show_demo_window = true;
		ImGui::ShowDemoWindow(&show_demo_window);
	}

	void BaseWindow::run() {

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		static float et = 0.f;
		while (not glfwWindowShouldClose(this->p_wnd))
		{
			auto beg = std::chrono::steady_clock::now();

			//
			glfwPollEvents();


			//
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();


			//
			this->update(et);


			//
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			//
			glfwSwapBuffers(this->p_wnd);


			//
			auto end = std::chrono::steady_clock::now();
			et = std::chrono::duration<float>(end - beg).count();
		}
	}

	void clear_screen(float r, float g, float b, float a)
	{
		GLLOGERROR(glClearColor(r, g, b, a));
		GLLOGERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}
}