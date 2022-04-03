#define __CL_ENABLE_EXCEPTIONS

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include "CL/cl.hpp"

#include "config.h"

#define GLOBAL_SIZE 36
#define LOCAL_SIZE 6

#define OUTFILE "out.txt"

#define KERNEL_DIRECTION "direction"
#define KERNEL_WATER "compute"

#define DECL_GET(__type) 																					\
	inline __type get(const __type *const array, const size_t &x, const size_t &y, const size_t &sz_x) { 	\
		return array[x * sz_x + y];																			\
	}

#define DECL_SET(__type) 																								\
	inline void set(__type *const array, const size_t &x, const size_t &y, const size_t &sz_x, const __type &value) {	\
		array[x * sz_x + y] = value;																					\
	}

#define DECL_PRINT(__type) 																									\
	inline void print_array(const __type *const array, const size_t &sz_x, const size_t &sz_y, const std::string &name) {	\
        std::clog << name << std::endl;                                            											\
		for (size_t x = 0; x < sz_x; ++x) {                                                									\
			std::cout << "x:" << x << " [ ";																				\
			for (size_t y = 0; y < sz_y; ++y)   																			\
                std::cout << "y:" << get(array, x, y, sz_x) << " ";                         								\
            std::cout << "]" << std::endl;                  																\
		}																													\
	}																														\

DECL_GET(float)
DECL_PRINT(float)
DECL_GET(int)
DECL_PRINT(int)

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

std::string get_arrow(const int &index) {
	switch (index) {
		default:
		case 0:
			return " ";
		case 1:
			return "↖";
		case 2:
			return "↑";
		case 3:
			return "↗";
		case 4:
			return "→";
		case 5:
			return "↘";
		case 6:
			return "↓";
		case 7:
			return "↙";
		case 8:
			return "←";
	}
}

void print_in_file(const int *const array, const size_t &sz_x, const size_t &sz_y, const std::string &nom, const bool &is_arrow) {
	std::ofstream file;
	file.open(OUTFILE, std::ios::app);
	file << nom << ":" << std::endl;
	for (size_t i = 0; i < sz_x; ++i) {
		for (size_t j = 0; j < sz_y; ++j)
			(is_arrow) ?
				file << get_arrow(get(array, i, j, sz_x)) << " " :
				file << get(array, i, j, sz_x) << " ";
		file << std::endl;
	}
	file.close();
}

bool has_zero(const int *const array, const size_t &sz_x, const size_t &sz_y) {
	for (size_t i = 0; i < sz_x * sz_y; ++i)
		if (array[i] == 0) return true;
	return false;
}

void GPU(const cl::Program &program, const cl::CommandQueue &queue, const cl::Context &context,
		 const float *const data, const size_t sz_x, const size_t sz_y, int *const directions, int *const water, const int &nodata) {

	// Size_t dosen't exist in OpenCL, so we have to convert it to int
	const int shrinked_sz_x = (int) sz_x;
	const int shrinked_sz_y = (int) sz_y;

	// Create buffers
	cl::Buffer data_buffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(float) * sz_x * sz_y);
	cl::Buffer direction_bufffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(int) * sz_x * sz_y);

	queue.enqueueWriteBuffer(data_buffer, CL_TRUE, 0, sizeof(float) * sz_x * sz_y, data);
	queue.enqueueWriteBuffer(direction_bufffer, CL_TRUE, 0, sizeof(int) * sz_x * sz_y, directions);

	print_array(data, sz_x, sz_y, "DATA");

	// Setup direction computation
	cl::Kernel kernel_direction(program, KERNEL_DIRECTION);
	kernel_direction.setArg(0, data_buffer);
	kernel_direction.setArg(1, direction_bufffer);
	kernel_direction.setArg(2, shrinked_sz_x);
	kernel_direction.setArg(3, shrinked_sz_y);
	kernel_direction.setArg(4, 8 * sizeof(float), nullptr);
	kernel_direction.setArg(5, nodata);
	//Processing elements
	cl::NDRange global(GLOBAL_SIZE);
	// Compute units
	cl::NDRange local(LOCAL_SIZE);

	// Start GPU
	queue.enqueueNDRangeKernel(kernel_direction, cl::NullRange, global, local);

	// Fetch result
	queue.enqueueReadBuffer(direction_bufffer, CL_TRUE, 0, sz_x * sz_y * sizeof(int), directions);

	print_array(directions, sz_x, sz_y, "DIRECTION");
	print_in_file(directions, sz_x, sz_y, "DIRECTION", true);

	// ######################################################################################################################


	// Create buffers
	cl::Buffer water_buffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(int) * sz_x * sz_y);
	cl::Buffer direction_buffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int) * sz_x * sz_y);
	queue.enqueueWriteBuffer(direction_buffer, CL_TRUE, 0, sizeof(int) * sz_x * sz_y, directions);
	queue.enqueueWriteBuffer(water_buffer, CL_TRUE, 0, sizeof(int) * sz_x * sz_y, water);

	// Setup water computation
	cl::Kernel kernel_water(program, KERNEL_WATER);
	kernel_water.setArg(0, water_buffer);
	kernel_water.setArg(1, direction_buffer);
	kernel_water.setArg(2, shrinked_sz_x);
	kernel_water.setArg(3, shrinked_sz_y);

	// While there is water flow to compute
	do {
		queue.enqueueNDRangeKernel(kernel_water, cl::NullRange, global, local);
		queue.enqueueReadBuffer(water_buffer, CL_TRUE, 0, sz_x * sz_y * sizeof(int), water);
	} while (has_zero(water, sz_x, sz_y));

	// Output results
	print_in_file(water, sz_x, sz_y, "WATER", false);
}

int main() {
	size_t sz_x = 0, sz_y = 0, left = 0, right = 0, cell = 0;
	int nodata = 0;
	float *data = nullptr;
	read_file(sz_x, sz_y, left, right, cell, nodata, &data);
	const size_t sz = sz_x * sz_y;
	int *directions = new int[sz], *water = new int[sz] { 0 };

	try {
		// Fetch platforms
		std::vector<cl::Platform> platforms;
		// Should be one on PC
		cl::Platform::get(&platforms);

		// Fetch associated devices
		std::vector<cl::Device> devices;
		platforms[0].getDevices(CL_DEVICE_TYPE_ALL, &devices);

		// Create context
		cl::Context context(devices);
		// Read program code
		cl::Program program = create_program(PROGRAM, context);
		try {
			// Compile program
			program.build(devices);
		} catch (...) {
			// Catch errors
			cl_int buildErr = CL_SUCCESS;
			auto buildInfo = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0], &buildErr);
			std::cerr << buildInfo << std::endl << std::endl;
			exit(1);
		}

		// Create queue and start GPU version
		cl::CommandQueue queue = cl::CommandQueue(context, devices[0]);
		GPU(program, queue, context, data, sz_x, sz_y, directions, water, nodata);

		// Free memory
		delete[] data;
		delete[] directions;
		delete[] water;

	} catch (const cl::Error &err) {
		std::cout << "Exception" << std::endl;
		std::cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << std::endl;
		return EXIT_FAILURE;
	}
}