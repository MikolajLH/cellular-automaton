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