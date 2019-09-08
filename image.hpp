#ifndef PPMPP_IMAGE_HPP
#define PPMPP_IMAGE_HPP
#include <stdexcept>
#include <vector>
#include <cstdlib>
#include <utility>
#include <algorithm>
#include <iostream>

#include "pixel.hpp"

namespace ppmpp {

template<typename P>
class image;

template<typename P>
class row {
public:
	P& operator[](uint16_t x) const {
		check_bounds(x);

		return pixels[x];
	}

	P& operator[](uint16_t x) {
		check_bounds(x);

		return pixels[x];
	}

private:
	void check_bounds(uint16_t x) const {
		if (x >= width)
			throw std::out_of_range("X coordinate is out of bounds");
	}

	friend class image<P>;

	row(P* pixels, size_t width) : pixels(pixels), width(width) {}

	P* pixels;
	uint16_t width;
};

template<typename P>
class image {
public:
	image(uint16_t width, uint16_t height, const P& color = P()) : width(width), height(height) {
		pixels.resize((size_t)(width * height), color);
		
		if (pixels.size() == 0)
			throw std::domain_error("Width and height of an image has to be non-zero");
	}

	row<P> operator[](size_t y) {
		check_bounds(y);

		return row(&pixels[y * width], width);
	}

	row<const P> operator[](size_t y) const {
		check_bounds(y);

		return row(&pixels[y * width], width);
	}

	uint16_t get_width() const {
		return width;
	}

	uint16_t get_height() const {
		return height;
	}

	std::vector<unsigned char> serialize_binary() const {
		auto buffer = make_header(true);

		if constexpr(std::is_same<bw_pixel, P>::value) {
			for (size_t y = 0; y < height; y++) {
				size_t done = 0;

				while (done < width) {
					unsigned char b = 0;
					auto n = std::min((size_t)8, width - done);

					for (size_t i = 0; i < n; i++, done++) 
						b |= (pixels[y * width + done].value ? 1 : 0) << (8 - i - 1);
					
					write_binary(buffer, (unsigned char)~b);
				}
			}
		} else {
			for (const auto& pixel : pixels) {
				for (size_t i = 0; i < detail::pixel_traits<P>::ELEMENT_COUNT; i++)
					write_binary(buffer, pixel[i]);
			}
		}
		
		return std::move(buffer);
	}

	std::vector<unsigned char> serialize_text() const {
		auto buffer = make_header(false);
		
		for (const auto& pixel : pixels) {
			for (size_t i = 0; i < detail::pixel_traits<P>::ELEMENT_COUNT; i++) {
				write_text(buffer, +pixel[i]);
				buffer.push_back((unsigned char)' ');
			}
		}
		
		return std::move(buffer);
	}

private:
	std::vector<unsigned char> make_header(bool binary) const {
		std::vector<unsigned char> buffer;

		buffer.push_back((unsigned char)'P');
		write_text(buffer, detail::pixel_traits<P>::ID + (binary ? 3 : 0));
		buffer.push_back((unsigned char)' ');

		write_text(buffer, width);
		buffer.push_back((unsigned char)' ');

		write_text(buffer, height);
		buffer.push_back((unsigned char)'\n');		

		if constexpr(detail::pixel_traits<P>::MAX_VALUE) {
			write_text(buffer, *detail::pixel_traits<P>::MAX_VALUE);
			buffer.push_back((unsigned char)'\n');
		}

		return std::move(buffer);
	}

	void check_bounds(uint16_t y) const {
		if (y >= height) 
			throw std::out_of_range("Y coordinate is out of bounds");
	}

	template<typename T>
	static void write_binary(std::vector<unsigned char>& buffer, T n) {
		for (size_t i = 0; i < sizeof(n); i++)  
			buffer.push_back((unsigned char)((n >> (i * 8)) & (T)0xFF));
	}

	template<typename T>
	static void write_text(std::vector<unsigned char>& buffer, T n) {
		if (n == 0) {
			buffer.push_back((unsigned char)'0');
			return;
		}

		size_t count = 0;
		unsigned char digits[128] = { 0 };
		while (n != 0) {
			digits[count++] = ((unsigned char)(n % 10) + (unsigned char)'0');
			n /= 10;
		}

		buffer.reserve(count);

		for (size_t i = 0; i < count; i++)
			buffer.push_back(digits[count - i - 1]);
	}

	uint16_t width;
	uint16_t height;
	std::vector<P> pixels;
};


}

#endif