__kernel
void direction(__global float *data, __global int *direction,
			   const int sz_x, const int sz_y,
			   __local float *neighbors, const int nodata) {

	// Get the current pixel position
	int id = get_global_id(0);
	int i = id / sz_x;
	int j = (id - (i * sz_x));
	int index = i * sz_x + j;

	// Current minimum
	float min = data[index];

	// If current pixel is nodata,
	// and set direction to 0 and skip it
	if ((int) min == nodata ) {
		direction[index] = 0;
		return;
	}

	// Init all neighbors to -1
	// This means that they are not set yet
	int index_min = -1;
	for (int k = 0; k < 8; ++k) {
		neighbors[k] = -1;
	}

	//NORD
	if (i > 0) {
		neighbors[1] = data[(i - 1) * sz_x + j]; // n id : 1
		if (j > 0) {
			neighbors[0] = data[(i - 1) * sz_x + (j - 1)]; // no id : 0
		}
		if (j < sz_y - 1) {
			neighbors[2] = data[(i - 1) * sz_x + (j + 1)]; // ne id : 2
		}
	}

	//SUD
	if (i < sz_x - 1) {
		neighbors[5] = data[(i + 1) * sz_x + j]; // s id : 5
		if (j > 0) {
			neighbors[6] = data[(i + 1) * sz_x + (j - 1)]; // so id : 6
		}
		if (j < sz_y - 1) {
			neighbors[4] = data[(i + 1) * sz_x + (j + 1)]; // se id : 4
		}
	}

	//Ouest
	if (j > 0) {
		neighbors[7] = data[index - 1]; // o id : 7
	}

	//Est
	if (j < sz_y - 1) {
		neighbors[3] = data[index + 1]; // e id : 3
	}

	// Find the minimum neighbor
	for (int k = 0; k < 8; ++k)
		if (neighbors[k] != -1 && ((int) neighbors[k]) != nodata && min > neighbors[k]) {
			min = neighbors[k];
			index_min = k;
		}
	// If no minimum neighbor is found, set the direction to 0
	// Else set the direction to the index of the minimum neighbor
	direction[index] = (index_min == -1) ? 0 : index_min + 1;
}

__kernel void compute(__global int *res, __global int *direction, const int nx, const int ny) {

	// Get the current pixel position
	int id = get_global_id(0);
	int i = id / nx;
	int j = (id - (i * nx));
	int index = i * nx + j;

	// Store the sum
	int sum = 0;

	// Compute the sum of all neighbors pointing to the current pixel
	// If one of the neighbors is no computed yet (res = 0), break the loop
	// To wait for it to be computed

	// NORD
	if (i > 0) {
		if (direction[(i - 1) * nx + j] == 6) {
			if (res[(i - 1) * nx + j] != 0)
				sum += res[(i - 1) * nx + j]; // N
			else return;
		}

		if (j > 0) {
			if (direction[(i - 1) * nx + (j - 1)] == 5) {
				if (res[(i - 1) * nx + (j - 1)] != 0)
					sum += res[(i - 1) * nx + (j - 1)]; // NO
				else return;
			}
		}

		if (j < ny - 1) {
			if (direction[(i - 1) * nx + (j + 1)] == 7) {
				if (res[(i - 1) * nx + (j + 1)] != 0)
					sum += res[(i - 1) * nx + (j + 1)]; // NE
				else return;
			}
		}
	}

	//SUD
	if (i < nx - 1) {
		if (direction[(i + 1) * nx + j] == 2) {
			if (res[(i + 1) * nx + j] != 0)
				sum += res[(i + 1) * nx + j]; // S
			else return;
		}

		if (j > 0) {
			if (direction[(i + 1) * nx + (j - 1)] == 3) {
				if (res[(i + 1) * nx + (j - 1)] != 0)
					sum += res[(i + 1) * nx + (j - 1)]; // SO
				else return;
			}
		}

		if (j < ny - 1) {
			if (direction[(i + 1) * nx + (j + 1)] == 1) {
				if (res[(i + 1) * nx + (j + 1)] != 0)
					sum += res[(i + 1) * nx + (j + 1)]; // SE
				else return;
			}
		}
	}

	//Ouest
	if (j > 0) {
		if (direction[index - 1] == 4) {
			if (res[index - 1] != 0)
				sum += res[index - 1]; // O
			else return;
		}
	}

	//Est
	if (j < ny - 1) {
		if (direction[index + 1] == 8) {
			if (res[index + 1] != 0)
				sum += res[index + 1]; // E
			else return;
		}
	}

	// Set the num sum + 1
	res[index] = ++sum;
}