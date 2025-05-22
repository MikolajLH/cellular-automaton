#pragma once
#include <vector>
#include <functional>
#include "GridView.hpp"


template<typename T>
class Automaton {
public:
	Automaton(size_t rows, size_t cols, const T& fill_v = T{})
		:
		R{ rows }, C{ cols },
		fg_index{ 0 } {

		data[0] = std::vector(rows * cols, fill_v);
		data[1] = std::vector(rows * cols, fill_v);
	}

	size_t rows() const noexcept { return R; }
	size_t cols() const noexcept { return C; }

	using SetUpFunction = std::function<void(GridView<T>)>;
	void setup(const SetUpFunction& fn) {
		fn(bg());
		fg_index = (fg_index + 1) % 2;
	}

	using UpdateFunction = std::function<void(GridView<const T>, GridView<T>)>;
	void update(const UpdateFunction& fn) {
		fn(fg(), bg());
		fg_index = (fg_index + 1) % 2;
	}

	auto fg() {
		return GridView<const T>(data[fg_index], rows(), cols());
	}

	auto bg() {
		return GridView<T>(data[(fg_index + 1) % 2], rows(), cols());
	}

private:
	size_t R;
	size_t C;

	size_t fg_index;
	std::vector<T>data[2];
};