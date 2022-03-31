#include <cstdlib>
#include <iostream>
#include "CL/cl.h"

#include "config.h"

#define DECL_GET(__type) 																					\
	inline __type get(const __type *const array, const size_t &x, const size_t &y, const size_t &sz_x) { 	\
		return array[x * sz_x + y];																			\
	}

#define DECL_SET(__type) 																								\
	inline void set(__type *const array, const size_t &x, const size_t &y, const size_t &sz_x, const __type &value) {	\
		array[x * sz_x + y] = value;																					\
	}

DECL_GET(float)
DECL_SET(float)
DECL_GET(uint8_t)

//void print_directions(const uint8_t *const directions, const size_t &sz_x, const size_t &sz_y) {
//	// Array aren't displayed
//	constexpr char arrows[] = "x↖↑↗→↘↓↙←";
//	for (size_t i = 0; i < sz_x; ++i) {
//		for (size_t j = 0; j < sz_y; ++j) std::cout << arrows[directions[i * sz_y + j]] << " ";
//		std::cout << std::endl;
//	}
//}

void read_file(size_t& sz_x, size_t& sz_y, size_t& left, size_t& right, size_t& cell, int& nodata, float **data) {
	FILE *fp = fopen(FILENAME, "r");
	if (fp == nullptr) exit(EXIT_FAILURE);

	fscanf(fp, "%zu", &sz_x);
	fscanf(fp, "%zu", &sz_y);
	fscanf(fp, "%zu", &left);
	fscanf(fp, "%zu", &right);
	fscanf(fp, "%zu", &cell);
	fscanf(fp, "%d", &nodata);

	*data = new float[sz_x * sz_y];
	float height;
	for (size_t i = 0; i < sz_x; ++i) {
		for (size_t j = 0; j < sz_y; ++j) {
			fscanf(fp, "%f", &height);
			(*data)[i * sz_x + j] = height;
		}
	}

	fclose(fp);
}

uint8_t *generator_direction(const size_t &sz_x, const size_t &sz_y, const float *const data, const int& nodata) {
	auto *directions = new uint8_t[sz_x * sz_y];
	const size_t max_x = sz_x - 1;
	const size_t max_y = sz_y - 1;

	for (size_t y = 0; y < sz_y; ++y)
		for (size_t x = 0; x < sz_x; ++x) {

			if (((int) get(data, x, y, sz_x)) == nodata) continue;

			float minimum_value = get(data, x, y, sz_x), current;
			uint8_t direction = 0;

			if (x > 0 && y > 0) {
				current = get(data, x - 1, y - 1, sz_x);
				if (current < minimum_value && ((int) current != nodata)) {
					direction = 1;
					minimum_value = current;
				}
			}

			if (x > 0) {
				current = get(data, x - 1, y, sz_x);
				if (current < minimum_value && ((int) current != nodata)) {
					direction = 2;
					minimum_value = current;
				}
			}

			if (x > 0 && y < max_y) {
				current = get(data, x - 1, y + 1, sz_x);
				if (current < minimum_value && ((int) current != nodata)) {
					direction = 3;
					minimum_value = current;
				}
			}

			if (y < max_y) {
				current = get(data, x, y + 1, sz_x);
				if (current < minimum_value && ((int) current != nodata)) {
					direction = 4;
					minimum_value = current;
				}
			}

			if (y < max_y && x < max_x) {
				current = get(data, x + 1, y + 1, sz_x);
				if (current < minimum_value && ((int) current != nodata)) {
					direction = 5;
					minimum_value = current;
				}
			}

			if (x < max_x) {
				current = get(data, x + 1, y, sz_x);
				if (current < minimum_value && ((int) current != nodata)) {
					direction = 6;
					minimum_value = current;
				}
			}

			if (x < max_x && y > 0) {
				current = get(data, x + 1, y - 1, sz_x);
				if (current < minimum_value && ((int) current != nodata)) {
					direction = 7;
					minimum_value = current;
				}
			}

			if (y > 0) {
				current = get(data, x, y - 1, sz_x);
				if (current < minimum_value && ((int) current != nodata)) direction = 8;
			}

			directions[x * sz_x + y] = direction;
		}

	return directions;
}

bool iteration(const uint8_t *const directions, float *const water, const size_t &sz_x, const size_t &sz_y) {
	bool hasChanged = false;

	float previous, sum;
	for (size_t i = 0; i < sz_x; ++i)
		for (size_t j = 0; j < sz_y; ++j) {

			previous = get(water, i, j, sz_x);
			sum = 1.f;

			LOG(std::clog << "[" << i << "; " << j << "](" << previous << ")";)

			if (i > 0 && j > 0)
				if (get(directions, i - 1, j - 1, sz_x) == 5) {
					LOG(std::clog << "+TL(" << get(water, i - 1, j - 1, sz_x) << ")";)
					sum += get(water, i - 1, j - 1, sz_x);
				}

			if (i > 0)
				if (get(directions, i - 1, j, sz_x) == 6) {
					LOG(std::clog << "+T(" << get(water, i - 1, j, sz_x) << ")";)
					sum += get(water, i - 1, j, sz_x);
				}

			if (i > 0 && j < sz_y - 1)
				if (get(directions, i - 1, j + 1, sz_x) == 7) {
					LOG(std::clog << "+TR(" << get(water, i - 1, j + 1, sz_x) << ")";)
					sum += get(water, i - 1, j + 1, sz_x);
				}

			if (j < sz_y - 1)
				if (get(directions, i, j + 1, sz_x) == 8) {
					LOG(std::clog << "+R(" << get(water, i, j + 1, sz_x) << ")";)
					sum += get(water, i, j + 1, sz_x);
				}

			if (i < sz_x - 1 && j < sz_y - 1)
				if (get(directions, i + 1, j + 1, sz_x) == 1) {
					LOG(std::clog << "+BR(" << get(water, i + 1, j + 1, sz_x) << ")";)
					sum += get(water, i + 1, j + 1, sz_x);
				}

			if (i < sz_x - 1)
				if (get(directions, i + 1, j, sz_x) == 2) {
					LOG(std::clog << "+B(" << get(water, i + 1, j, sz_x) << ")";)
					sum += get(water, i + 1, j, sz_x);
				}

			if (i < sz_x - 1 && j > 0)
				if (get(directions, i + 1, j - 1, sz_x) == 3) {
					LOG(std::clog << "+BL(" << get(water, i + 1, j - 1, sz_x) << ")";)
					sum += get(water, i + 1, j - 1, sz_x);
				}

			if (j > 0)
				if (get(directions, i, j - 1, sz_x) == 4) {
					LOG(std::clog << "+L(" << get(water, i, j - 1, sz_x) << ")";)
					sum += get(water, i, j - 1, sz_x);
				}

			set(water, i, j, sz_x, sum);
			LOG(std::clog << "+1=" << sum << std::endl;)

			if (sum != previous) hasChanged = true;
		}

	return hasChanged;
}

int main() {

	size_t sz_x = 0, sz_y = 0, left = 0, right = 0, cell = 0;
	int nodata = 0;
	float *data = nullptr;
	read_file(sz_x, sz_y, left, right, cell, nodata, &data);

	LOG(
		std::clog << "sz_x: " << sz_x << std::endl;
		std::clog << "sz_y: " << sz_y << std::endl;
		std::clog << "left: " << left << std::endl;
		std::clog << "right: " << right << std::endl;
		std::clog << "cell: " << cell << std::endl;
		std::clog << "nodata: " << nodata << std::endl;

		std::clog << "data:" << std::endl;
		for (size_t i = 0; i < sz_x; ++i) {
			std::clog << i << ": [ ";
			for (size_t j = 0; j < sz_y; ++j)
				std::clog << get(data, i, j, sz_x) << " ";
			std::clog << "]" << std::endl;
		}
	)

	uint8_t *directions = generator_direction(sz_x, sz_y, data, nodata);

	LOG(
		std::clog << "directions:" << std::endl;
		for (size_t i = 0; i < sz_x; ++i) {
			std::clog << i << ": [ ";
			for (size_t j = 0; j < sz_y; ++j)
				std::clog << (int) get(directions, i, j, sz_x) << " ";
			std::clog << "]" << std::endl;
		}
	)

	delete[] data;

	auto *water = new float[sz_x * sz_y] { 0 };

	bool hasChanged;
	do {
		hasChanged = iteration(directions, water, sz_x, sz_y);
	} while (hasChanged);

	LOG(
		std::clog << "Water:" << std::endl;
		for (size_t i = 0; i < sz_x; ++i) {
			std::clog << i << ": [ ";
			for (size_t j = 0; j < sz_y; ++j)
				std::clog << get(water, i, j, sz_x) << " ";
			std::clog << "]" << std::endl;
		}
	)

	delete[] directions;
	delete[] water;

	return EXIT_SUCCESS;
}