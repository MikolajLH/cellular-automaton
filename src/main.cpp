#include "graphics/BaseWindow.h"
#include "graphics/Grid2d.h"
#include "graphics/primitives.h"

#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <concepts>
#include <ranges>
#include <cassert>

using ssize_t = std::ptrdiff_t;
template<typename T>
class GridView
{
public:
	template<std::ranges::contiguous_range Range>
	GridView(Range&& range, size_t r, size_t c)
		:
		R{ r }, C{ c },
		data(std::ranges::begin(range), std::ranges::end(range)) {
		assert(r * c == std::ranges::size(range));
	}

	size_t rows() { return R; }
	size_t cols() { return C; }

	auto& get_or(ssize_t r, ssize_t c, T& val) {
		if (r >= 0 and r < rows() and c >= 0 and c < cols())
			return data[r * cols() + c];
		return val;
	}

	//periodic boundry
	auto& get_pb(ssize_t r, ssize_t c) {
		r = (rows() + r) % rows();
		c = (cols() + c) % cols();
		return data[r * cols() + c];
	}

private:
	size_t R, C;
	std::span<T> data;
};

class Demo : public wnd::BaseWindow {

public:
	Grid2d grid;

	enum class CellState{
		DEAD,
		ALIVE,
	};

	std::vector<CellState> fg;
	std::vector<CellState> bg;

	std::unordered_map<CellState, glm::vec3>state_to_color;

	Demo(size_t rs, size_t cs)
		:
		BaseWindow(800, 600, "Demo!"),
		grid(rs, cs, primitives::P2::circle),
		fg(rs * cs, CellState::DEAD),
		bg(rs * cs, CellState::DEAD)
	{ 
		grid.scale = glm::vec2(10.f, 10.f);
		grid.change_and_update_interspace(glm::vec2(10.f, 10.f));

		state_to_color[CellState::DEAD] = glm::vec3(1.f, 0.f, 0.f);
		state_to_color[CellState::ALIVE] = glm::vec3(1.f, 0.f, 1.f);
	}

	void automaton_update(std::span<int>alive_rule, std::span<int> dead_rule) {

	}

	void update(float dt) override {
		
		const auto fw = this->get_width();
		const auto fh = this->get_height();

		wnd::clear_screen(0.5f, 0.3f, 0.6f);

		grid.draw(glm::ortho(-0.5f * fw, 0.5f * fw, -0.5f * fh, 0.5f * fh));

		ImGui::ShowDemoWindow();

		ImGui::Begin("control panel");

		ImGui::DragFloat("translation x", &grid.origin.x, 1.f);
		ImGui::DragFloat("translation y", &grid.origin.y, 1.f);
		ImGui::DragFloat("scale x", &grid.scale.x, 1.f);
		ImGui::DragFloat("scale y", &grid.scale.y, 1.f);

		ImGui::DragFloat("interspace x", &grid.interspace.x, 1.f);
		ImGui::DragFloat("interspace y", &grid.interspace.y, 1.f);

		ImGui::End();
		grid.change_and_update_interspace(grid.interspace);
		//ImGui::ShowDemoWindow();
	}
};


int main() {
	Demo window(50, 50);
	window.run();

	return 0;
}