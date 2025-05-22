#include "graphics/BaseWindow.h"

class Demo : public wnd::BaseWindow {

public:
	Demo()
		:
		BaseWindow(800, 600, "Demo!")
	{ }

	void update(float dt) override {
		wnd::clear_screen(0.5f, 0.3f, 0.6f);
		ImGui::ShowDemoWindow();
	}
};


int main() {
	Demo window;
	window.run();

	return 0;
}