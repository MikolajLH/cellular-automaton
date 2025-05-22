#include <unordered_map>
#include <algorithm>
#include <random>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "graphics/BaseWindow.h"
#include "graphics/Grid2d.h"
#include "graphics/primitives.h"

#include "GridView.hpp"
#include "Automaton.hpp"

bool coinflip(int percentage) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(1, 100);

	return dist(gen) <= percentage;

}

class Demo : public wnd::BaseWindow {

public:
	Grid2d grid;
	

	enum class CellState{
		DEAD,
		ALIVE,
	};
	std::unordered_map<CellState, glm::vec3>state_to_color{};

	Automaton<CellState> automaton;

	size_t R;
	size_t C;


	Demo(size_t rs, size_t cs)
		:
		BaseWindow(800, 600, "Demo!"),
		grid(rs, cs, primitives::P2::circle),
		automaton(rs, cs),
		R{ rs },
		C{ cs }
	{ 
		grid.scale = glm::vec2(10.f, 10.f);
		grid.change_and_update_interspace(glm::vec2(0.f, 0.f));

		state_to_color[CellState::DEAD] = glm::vec3(1.f, 0.f, 0.f);
		state_to_color[CellState::ALIVE] = glm::vec3(1.f, 0.f, 1.f);

		automaton.setup(reroll(40));
	}

	static auto reroll(int alive_chance) -> Automaton<CellState>::SetUpFunction {
		return [alive_chance](GridView<CellState> gd) {
			for (size_t r = 0; r < gd.rows(); ++r)
				for (size_t c = 0; c < gd.cols(); ++c) {
					gd.get_pb(r, c) = coinflip(alive_chance) ? CellState::ALIVE : CellState::DEAD;
				}
			};
	}

	void automaton_update(std::span<int>alive_rule, std::span<int> dead_rule) {

		automaton.update(
			[alive_rule, dead_rule](GridView<const CellState> fg, GridView<CellState> bg) {

			auto cell_val = [](CellState st) -> int {
				switch (st) {
				case CellState::DEAD: return 0;
				case CellState::ALIVE: return 1;
				};
				return 0;
				};

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
			});
	}

	void update_grid() {
		for(size_t r = 0; r < R; ++r)
			for (size_t c = 0; c < C; ++c) {
				grid.change_color(r, c, state_to_color[automaton.fg().get_pb(r, c)]);
			}
		grid.update_colors();
	}

	void parse_mode(std::string_view mode, std::vector<int>& alive, std::vector<int>& dead) {
		if (not std::ranges::contains(mode, '/'))
			return;


		alive.clear();
		for (auto c : std::views::take_while(mode, [](char k) { return k != '/'; })) {
			if (c >= '0' and c <= '9')
				alive.push_back(static_cast<int>(c - '0'));
		}

		dead.clear();
		for (auto c : std::views::drop_while(mode, [](char k) { return k != '/'; })) {
			if (c >= '0' and c <= '9')
				dead.push_back(static_cast<int>(c - '0'));
		}
	}

	void update(float dt) override {
		static float et = 0.f;
		et += dt;
		
		static std::vector<int> alive{ 2,3 };
		static std::vector<int> dead{ 3 };

		static float period = 0.2f;
		static bool play = false;
		if (play and et > period) {
			et = 0.f;
			automaton_update(alive, dead);
			update_grid();
		}
	
		const auto fw = this->get_width();
		const auto fh = this->get_height();

		auto dcv = state_to_color[CellState::DEAD];
		static float dead_col[3] = { dcv.r, dcv.g, dcv.b };

		auto acv = state_to_color[CellState::ALIVE];
		static float alive_col[3] = { acv.r, acv.g, acv.b };

		wnd::clear_screen(0.5f, 0.3f, 0.6f);
		
		grid.draw(glm::ortho(-0.5f * fw, 0.5f * fw, -0.5f * fh, 0.5f * fh));

		ImGui::ShowDemoWindow();

		ImGui::Begin("control panel");

		ImGui::DragFloat("translation x", &grid.origin.x, 1.f);
		ImGui::DragFloat("translation y", &grid.origin.y, 1.f);

		ImGui::InputFloat("scale x", &grid.scale.x, 0.5f, 1.f);
		ImGui::InputFloat("scale y", &grid.scale.y, 0.5f, 1.f);

		ImGui::InputFloat("interspace x", &grid.interspace.x, 0.5f, 1.f);
		ImGui::InputFloat("interspace y", &grid.interspace.y, 0.5f, 1.f);

		ImGui::InputFloat("update period", &period, 0.05f, 0.2f);


		ImGui::ColorEdit3("DEAD color", dead_col);
		state_to_color[CellState::DEAD] = glm::vec3(dead_col[0], dead_col[1], dead_col[2]);
		ImGui::ColorEdit3("ALIVE color", alive_col);
		state_to_color[CellState::ALIVE] = glm::vec3(alive_col[0], alive_col[1], alive_col[2]);

		static char buff[20] = "23/3";
		auto buff_sv = std::string_view(buff, 20);

		ImGui::InputText("mode", buff, 20, ImGuiInputTextFlags_CharsDecimal);
		parse_mode(buff_sv, alive, dead);

		float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
		if (ImGui::Button(play ? "stop" : "play")) {
			play = not play;

		}
		if (not play) {
			ImGui::SameLine(0.0f, spacing);
			if (ImGui::Button("next frame")) {
				automaton_update(alive, dead);
				update_grid();
			}
		}

		spacing = ImGui::GetStyle().ItemInnerSpacing.x;
		static int alive_chance = 40;
		ImGui::DragInt("alive chance", &alive_chance, 1, 0, 100, "%d%%", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SameLine(0.0f, spacing);
		{
			if (ImGui::Button("reroll")) {
				automaton.setup(reroll(100 - alive_chance));
				update_grid();
			}
		}

		ImGui::End();
		grid.change_and_update_interspace(grid.interspace);
	}
};


int main() {
	Demo window(200, 200);
	window.run();

	return 0;
}