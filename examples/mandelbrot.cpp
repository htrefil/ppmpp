#include <iostream>
#include <complex>
#include <cstring>
#include <cstdlib>
#include <fstream>

#include "../image.hpp"

static constexpr uint16_t WIDTH = 512;
static constexpr uint16_t HEIGHT = 512;

static float iter(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
	const int MAX_ITERATIONS = 200;

	std::complex c(
		(float)x * 2.0f / (float)width - 1.5f,
		(float)y * 2.0f / (float)height - 1.0f
	);

	std::complex z(0.0f, 0.0f);

	for (int i = 0; i < MAX_ITERATIONS; i++) {
		z = z * z + c;
		if (std::abs(z) > 2.0)
			return (float)i / (float)MAX_ITERATIONS;
	}

	return 0.0;
}

template<typename P>
static P to_pixel(float val);

template<>
ppmpp::bw_pixel to_pixel(float val) {
	return ppmpp::bw_pixel(val != 0.0);
}

template<>
ppmpp::rgb8_pixel to_pixel(float val) {
	return ppmpp::rgb8_pixel((uint8_t)(val * 255.0), 0, 0); 
}

template<>
ppmpp::rgb16_pixel to_pixel(float val) {
	return ppmpp::rgb16_pixel((uint16_t)(val * 65535.0), 0, 0); 
}

template<>
ppmpp::grayscale8_pixel to_pixel(float val) {
	return ppmpp::grayscale8_pixel((uint8_t)(val * 255.0)); 
}

template<>
ppmpp::grayscale16_pixel to_pixel(float val) {
	return ppmpp::grayscale16_pixel((uint16_t)(val * 65535.0)); 
}

template<typename P>
static std::vector<unsigned char> run(bool binary) {
	ppmpp::image<P> image(WIDTH, HEIGHT);

	for (uint16_t y = 0; y < HEIGHT; y++) {
		for (uint16_t x = 0; x < WIDTH; x++) {
			image[y][x] = to_pixel<P>(iter(x, y, WIDTH, HEIGHT));
		}
	}

	return binary ? image.serialize_binary() : image.serialize_text();
}

static const struct {
	const char* name;
	std::vector<unsigned char>(*func)(bool);
} TYPES[] = {
	{ "bw", run<ppmpp::bw_pixel> },
	{ "grayscale8", run<ppmpp::grayscale8_pixel> },
	{ "grayscale16", run<ppmpp::grayscale16_pixel> },
	{ "rgb8", run<ppmpp::rgb8_pixel> },
	{ "rgb16", run<ppmpp::rgb16_pixel> },
};

int main(int argc, char** argv) {
	if (argc != 4) {
		std::cout << "Usage: " << argv[0] << " <binary | text> <output type> <output path>" << std::endl;
		std::cout << "Available output types are: " << std::endl;
		for (const auto& type : TYPES)
			std::cout << "  " << type.name << std::endl;

		return EXIT_FAILURE;
	}

	auto output_type = argv[1];
	bool binary;
	if (strcmp(output_type, "binary") == 0) {
		binary = true;
	} else if (strcmp(output_type, "text") == 0) {
		binary = false;
	} else {
		std::cout << "Error: Invalid output type" << std::endl;
		return EXIT_FAILURE;
	}

	auto color_type = argv[2];
	for (const auto& type : TYPES) {
		if (strcmp(color_type, type.name) == 0) {
			auto data = type.func(binary);

			try {
				std::ofstream file;
				file.exceptions(std::ofstream::badbit);
				file.open(argv[3]);
				file.write((const char*)data.data(), data.size());	
			} catch (const std::ofstream::failure&) {
				std::cout << "Error writing to file" << std::endl;
				return EXIT_FAILURE;
			}
			
			return EXIT_SUCCESS;
		}
	}	

	std::cout << "Error: Invalid color type" << std::endl;
	return EXIT_FAILURE;
}