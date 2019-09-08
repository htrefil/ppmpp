#ifndef PPMPP_PIXEL_HPP
#define PPMPP_PIXEL_HPP
#include <cstdint>
#include <type_traits>
#include <limits>
#include <optional>

namespace ppmpp {

template<typename P>
class image;

class bw_pixel {
public:
	using type = bool;

	bw_pixel() : value(false) {}

	bw_pixel(bool value) : value(value) {}

	const bool& operator[](size_t idx) const {
		if (idx != 0)
			out_of_bounds();

		return value;
	}

	bool& operator[](size_t idx) {
		if (idx != 0)
			out_of_bounds();

		return value;
	}

	bool value;

private:
	static void out_of_bounds() {
		throw std::out_of_range("Black and white pixel value index out of range");
	}
};

template<typename T>
class grayscale_pixel {
public:
	static_assert(std::is_unsigned_v<T>);

	using type = T;

	grayscale_pixel() : value(0) {}

	grayscale_pixel(T value) : value(value) {}

	const T& operator[](size_t idx) const {
		if (idx != 0)
			out_of_bounds();

		return value;
	}

	T& operator[](size_t idx) {
		if (idx != 0)
			out_of_bounds();

		return value;
	}

	T value;

private:
	static void out_of_bounds() {
		throw std::out_of_range("Grayscale pixel value index out of range");
	}
};

using grayscale8_pixel = grayscale_pixel<uint8_t>;
using grayscale16_pixel = grayscale_pixel<uint16_t>;

template<typename T>
class rgb_pixel {
public:
	static_assert(std::is_unsigned_v<T>);

	using type = T;

	rgb_pixel() : r(0), g(0), b(0) {}

	rgb_pixel(T r, T g, T b) : r(r), g(g), b(b) {}

	const T& operator[](size_t idx) const {
		switch (idx) {
			case 0:
				return r;

			case 1:
				return g;

			case 2:
				return b;

			default:
				out_of_bounds();
		}
	}

	T& operator[](size_t idx) {
		switch (idx) {
			case 0:
				return r;

			case 1:
				return g;

			case 2:
				return b;

			default:
				out_of_bounds();
		}
	}

	T r;
	T g;
	T b;

private:
	[[noreturn]]
	static void out_of_bounds() {
		throw std::out_of_range("RGB value index out of range");
	}
};

using rgb8_pixel = rgb_pixel<uint8_t>;
using rgb16_pixel = rgb_pixel<uint16_t>;

namespace detail {

template<typename P>
class pixel_traits {};

template<>
struct pixel_traits<bw_pixel> {
public:
	static constexpr int ID = 1;
	static constexpr size_t ELEMENT_COUNT = 1;
	static constexpr std::optional<typename bw_pixel::type> MAX_VALUE = {};
};

template<typename T>
struct pixel_traits<grayscale_pixel<T>> {
public:
	static constexpr int ID = 2;
	static constexpr size_t ELEMENT_COUNT = 1;
	static constexpr std::optional<typename grayscale_pixel<T>::type> MAX_VALUE = std::numeric_limits<typename grayscale_pixel<T>::type>::max();
};

template<typename T>
struct pixel_traits<rgb_pixel<T>> {
public:
	static constexpr int ID = 3;
	static constexpr size_t ELEMENT_COUNT = 3;
	static constexpr std::optional<typename grayscale_pixel<T>::type> MAX_VALUE = std::numeric_limits<typename rgb_pixel<T>::type>::max();
};

}

}

#endif