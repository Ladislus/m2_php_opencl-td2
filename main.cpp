#include <cstdlib>
#include <iostream>
#include <limits>
#include "CL/cl.h"

#include "config.h"

void read_file(size_t& sz_x, size_t& sz_y, size_t& left, size_t& right, size_t& cell, int& nodata, float *data) {
	FILE *fp = fopen(FILENAME, "r");
	if (fp == nullptr) exit(EXIT_FAILURE);

	fscanf(fp, "%zu", &sz_x);
	fscanf(fp, "%zu", &sz_y);
	fscanf(fp, "%zu", &left);
	fscanf(fp, "%zu", &right);
	fscanf(fp, "%zu", &cell);
	fscanf(fp, "%d", &nodata);

	data = new float[sz_x * sz_y];
	float height;
	for (size_t i = 0; i < sz_x; ++i) {
		for (size_t j = 0; j < sz_y; ++j) {
			fscanf(fp, "%f", &height);
			data[i * sz_x + j] = height;
		}
	}

	//LOG(
	//	for(size_t i = 0; i < sz_x; ++i) {
	//		std::clog << "\ti: " << i << " [ ";
	//		for(size_t j = 0; j < sz_y; ++j) std::clog << data[i * sz_y + j] << " ";
	//		std::clog << "]" << std::endl;
	//	}
	//)

	fclose(fp);
}

inline float get(const float *const array, const size_t &x, const size_t &y, const size_t &sz_x) {
	return array[x * sz_x + y];
}

uint8_t *generator_direction(const size_t &sz_x, const size_t &sz_y, const float *const data) {
	auto *directions = new uint8_t[sz_x * sz_y];
	const size_t max_x = sz_x - 1;
	const size_t max_y = sz_y - 1;

	for (size_t y = 0; y < sz_y; ++y)
		for (size_t x = 0; x < sz_x; ++x) {

			float max = std::numeric_limits<float>::min();
			uint8_t direction = 0;

			if (x > 0 && y > 0 && get(data, x - 1, y - 1, sz_x) > max) {
				direction = 1;
				max = get(data, x - 1, y - 1, sz_x);
			}

			if (x > 0 && get(data, x - 1, y, sz_x) > max) {
				direction = 2;
				max = get(data, x - 1, y, sz_x);
			}

			if (x > 0 && y < max_y && get(data, x - 1, y + 1, sz_x) > max) {
				direction = 3;
				max = get(data, x - 1, y + 1, sz_x);
			}

			if (y < max_y && get(data, x, y + 1, sz_x) > max) {
				direction = 4;
				max = get(data, x, y + 1, sz_x);
			}

			if (y < max_y && x < max_x && get(data, x + 1, y + 1, sz_x) > max) {
				direction = 5;
				max = get(data, x + 1, y + 1, sz_x);
			}

			if (x < max_x && get(data, x + 1, y, sz_x) > max) {
				direction = 6;
				max = get(data, x + 1, y, sz_x);
			}

			if (x < max_x && y > 0 && get(data, x + 1, y - 1, sz_x) > max) {
				direction = 7;
				max = get(data, x + 1, y - 1, sz_x);
			}

			if (y > 0 && get(data, x, y - 1, sz_x) > max) direction = 8;

			CHECK(direction == 0, "No direction found")
			directions[x * sz_x + y] = direction;
		}

	return directions;
}

int main() {

	size_t sz_x = 0, sz_y = 0, left = 0, right = 0, cell = 0;
	int nodata = 0;
	float *data = nullptr;
	read_file(sz_x, sz_y, left, right, cell, nodata, data);

	LOG(
		std::clog << "sz_x: " << (sz_x) << std::endl;
		std::clog << "sz_y: " << (sz_y) << std::endl;
		std::clog << "left: " << (left) << std::endl;
		std::clog << "right: " << (right) << std::endl;
		std::clog << "cell: " << (cell) << std::endl;
		std::clog << "nodata: " << (nodata) << std::endl;
	)

	uint8_t *directions = generator_direction(sz_x, sz_y, data);

	delete[] data;
	delete[] directions;

	return EXIT_SUCCESS;
}