#define __CL_ENABLE_EXCEPTIONS

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "CL/cl.hpp"

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
DECL_GET(uint8_t)

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

cl::Program create_program(const std::string& filename, const cl::Context& context) {
	// Read source file
	std::ifstream source_file(filename);
	// Create a string containing the source code
	std::string source_code(std::istreambuf_iterator<char>(source_file), (std::istreambuf_iterator<char>()));
	// First argument is the number of used programs, the second is a pair (text, matrix_sz of the program)
	cl::Program::Sources source(1, std::make_pair(source_code.c_str(), source_code.length() + 1));
	// Return the program
	return { context, source };
}

uint8_t* compute_directions(const cl::Program &program, const cl::CommandQueue &queue, const cl::Context &context, const size_t &sz_x, const size_t &sz_y, const int &nodata, float *data) {
	// Define size constants
	const size_t sz_float = sizeof(float) * sz_x * sz_y;
	const size_t sz_uint8 = sizeof(uint8_t) * sz_x * sz_y;

	// Allocate memory for the result
	auto *const directions = new uint8_t[sz_x * sz_y];

	// Create buffers
	cl::Buffer buffer_data(context, CL_MEM_READ_ONLY, sz_float);
	cl::Buffer buffer_directions(context, CL_MEM_WRITE_ONLY, sz_uint8);

	// Copy data to the device
	queue.enqueueWriteBuffer(buffer_data, CL_TRUE, 0, sz_float, data);

	// Create directions_kernel
	cl::Kernel directions_kernel(program, "directions");

	// Set directions_kernel arguments
	directions_kernel.setArg(0, sz_x);
	directions_kernel.setArg(1, sz_y);
	directions_kernel.setArg(2, nodata);
	directions_kernel.setArg(3, buffer_data);
	directions_kernel.setArg(4, buffer_directions);

	// Create topology
	cl::NDRange global(sz_x, sz_y);  // Processing elements
	cl::NDRange local(16, 16); 		// Compute units

	// Launch program
	queue.enqueueNDRangeKernel(directions_kernel, cl::NullRange, global, local);

	// Read result from the device
	queue.enqueueReadBuffer(buffer_directions, CL_TRUE, 0, sz_uint8, directions);

	// Return the result
	return directions;
}

const float* compute(const cl::Program &program, const cl::CommandQueue &queue, const cl::Context &context, const size_t &sz_x, const size_t &sz_y, const uint8_t *const directions) {
	const size_t sz_float = sizeof(float) * sz_x * sz_y;
	const size_t sz_uint8 = sizeof(uint8_t) * sz_x * sz_y;

	auto *water = new float[sz_x * sz_y] { 0.f };

	// Create buffers
	cl::Buffer buffer_directions(context, CL_MEM_READ_ONLY, sz_uint8);
	cl::Buffer buffer_water(context, CL_MEM_READ_WRITE, sz_float);
	cl::Buffer buffer_hasChanged(context, CL_MEM_READ_WRITE, sizeof(bool));

	// Copy data to the device
	queue.enqueueWriteBuffer(buffer_directions, CL_TRUE, 0, sz_uint8, directions);
	queue.enqueueWriteBuffer(buffer_water, CL_TRUE, 0, sz_float, water);

	// Create compute_kernel
	cl::Kernel compute_kernel(program, "compute");

	// Set compute_kernel arguments
	compute_kernel.setArg(0, sz_x);
	compute_kernel.setArg(1, sz_y);
	compute_kernel.setArg(2, buffer_directions);
	compute_kernel.setArg(3, buffer_water);
	compute_kernel.setArg(4, buffer_hasChanged);

	// Create topology
	cl::NDRange global(sz_x, sz_y);  // Processing elements
	cl::NDRange local(16, 16); 		// Compute units

	bool hasChanged;
	do {
		// Reset hasChanged
		queue.enqueueReadBuffer(buffer_hasChanged, CL_TRUE, 0, sizeof(bool), &hasChanged);
		// Launch program
		queue.enqueueNDRangeKernel(compute_kernel, cl::NullRange, global, local);
		// Read result from the device (hasChanged)
		queue.enqueueReadBuffer(buffer_hasChanged, CL_TRUE, 0, sizeof(bool), &hasChanged);
	} while (hasChanged);

	return water;
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

	try {
		// Get platforms (should be unique on PC)
		std::vector<cl::Platform> plateformes;
		cl::Platform::get(&plateformes);

		// Get devices associated with the first platform (Should be unique on PC)
		std::vector<cl::Device> devices;
		plateformes[0].getDevices(CL_DEVICE_TYPE_ALL, &devices);

		// Create associated context
		cl::Context contexte(devices);

		// Create program
		cl::Program program = create_program(PROGRAM, contexte);

		// Build program (compilation)
		try {
			program.build(devices);
		} catch (...) {
			// Catch all possible errors and exit with code 1
			cl_int buildErr = CL_SUCCESS;
			auto buildInfo = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0], &buildErr);
			std::cerr << buildInfo << std::endl << std::endl;
			exit(1);
		}

		// Create the queue to communicate with the kernel
		cl::CommandQueue queue = cl::CommandQueue(contexte, devices[0]);

		uint8_t *directions = compute_directions(program, queue, contexte, sz_x, sz_y, nodata, data);
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

		//auto water = compute(program, queue, contexte, sz_x, sz_y, directions);

		//LOG(
		//	std::clog << "Water:" << std::endl;
		//	for (size_t i = 0; i < sz_x; ++i) {
		//		std::clog << i << ": [ ";
		//		for (size_t j = 0; j < sz_y; ++j)
		//			std::clog << get(water, i, j, sz_x) << " ";
		//		std::clog << "]" << std::endl;
		//	}
		//)

		delete[] directions;
		//delete[] water;
	} catch (const cl::Error& err) {
		std::cout << "Exception\n";
		std::cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << std::endl;
		return EXIT_FAILURE;
	}
}