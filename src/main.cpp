#include <unordered_map>
#include <algorithm>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "graphics/BaseWindow.h"
#include "graphics/Grid2d.h"
#include "graphics/primitives.h"

#include "GridView.hpp"


class Demo : public wnd::BaseWindow {

public:
	Grid2d grid;

	enum class CellState{
		DEAD,
		ALIVE,
	};

	static int cell_val(CellState state) {
		switch (state) {
		case CellState::DEAD: return 0;
		case CellState::ALIVE: return 1;
		};
		return 0;
	}

	std::vector<CellState> fgv;
	std::vector<CellState> bgv;
	bool turn;
	size_t R;
	size_t C;

	std::unordered_map<CellState, glm::vec3>state_to_color;

	Demo(size_t rs, size_t cs)
		:
		BaseWindow(800, 600, "Demo!"),
		grid(rs, cs, primitives::P2::quad),
		fgv(rs * cs, CellState::DEAD),
		bgv(rs * cs, CellState::DEAD),
		R{ rs },
		C{ cs },
		turn{}
	{ 
		grid.scale = glm::vec2(10.f, 10.f);
		grid.change_and_update_interspace(glm::vec2(10.f, 10.f));

		state_to_color[CellState::DEAD] = glm::vec3(1.f, 0.f, 0.f);
		state_to_color[CellState::ALIVE] = glm::vec3(1.f, 0.f, 1.f);
		
		for (size_t r = 0; r < R; ++r)
			for (size_t c = 0; c < C; ++c) {
				auto state = (rand() % 10) < 6 ? CellState::ALIVE : CellState::DEAD;
				fgv[r * C + c] = state;
				bgv[r * C + c] = state;
				//grid.change_color(r, c, state_to_color[fg.get_pb(r, c)]);
			}
	}

	void automaton_update(std::span<int>alive_rule, std::span<int> dead_rule) {
		auto fg = GridView<CellState>(turn ? fgv : bgv, R, C);
		auto bg = GridView<CellState>(turn ? bgv : fgv, R, C);

		for (size_t r = 0; r < fg.rows(); ++r)
			for (size_t c = 0; c < fg.cols(); ++c) {
				int s =
					cell_val(fg.get_pb(r + 1, c)) +
					cell_val(fg.get_pb(r - 1, c)) +
					cell_val(fg.get_pb(r, c + 1)) +
					cell_val(fg.get_pb(r, c - 1)) +

					cell_val(fg.get_pb(r + 1, c + 1)) +
					cell_val(fg.get_pb(r + 1, c - 1)) +
					cell_val(fg.get_pb(r - 1, c + 1)) +
					cell_val(fg.get_pb(r - 1, c - 1));

				if (fg.get_pb(r, c) == CellState::ALIVE) {

					bg.get_pb(r, c) = std::ranges::contains(alive_rule, s) ? CellState::ALIVE : CellState::DEAD;
				}
				else {
					bg.get_pb(r, c) = std::ranges::contains(dead_rule, s) ? CellState::ALIVE : CellState::DEAD;
				}
			}

		turn = not turn;
	}

	void update_grid() {
		auto fg = GridView<CellState>(turn ? fgv : bgv, R, C);
		auto bg = GridView<CellState>(turn ? bgv : fgv, R, C);
		for(size_t r = 0; r < R; ++r)
			for (size_t c = 0; c < C; ++c) {
				grid.change_color(r, c, state_to_color[fg.get_pb(r, c)]);
			}
		grid.update_colors();
	}

	void update(float dt) override {
		
		const auto fw = this->get_width();
		const auto fh = this->get_height();

		static std::vector<int> alive{ 2,3 };
		static std::vector<int> dead{ 3 };

		wnd::clear_screen(0.5f, 0.3f, 0.6f);
		automaton_update(alive, dead);
		update_grid();
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
	}
};


int main() {
	Demo window(50, 50);
	window.run();

	return 0;
}