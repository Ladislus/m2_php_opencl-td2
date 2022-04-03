__kernel
void direction(__global float *data, __global int *direction,
			   const int sz_x, const int sz_y,
			   __local float *neighbors, const int nodata) {
	int idT = get_global_id(0);
	int i = idT / sz_x;
	int j = (idT - (i * sz_x));
	int val = 0;
	int index = i * sz_x + j;
	float min = data[index];
	if ( (int)min == nodata ) {
		direction[index] = 0;
		return;
	}
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
	}//Est
	if (j < sz_y - 1) {
		neighbors[3] = data[index + 1]; // e id : 3
	}
	for (int k = 0; k < 8; ++k) {
// Pour chaque neighbors
		if (neighbors[k] != -1 && (int)neighbors[k] != nodata && min > neighbors[k]) {
			min = neighbors[k];
			index_min = k;
		}
	}
	direction[index] = (index_min == -1) ? 0 : index_min + 1;
}

__kernel void compute(__global int *res, __global int *direction, const int nx, const int ny) {
	int idT = get_global_id(0);
	int i = idT / nx;
	int j = (idT - (i * nx));
	int index = i * nx + j;
	int val = 0;
	if (i > 0) {
		if (direction[(i - 1) * nx + j] == 6) {
			if (res[(i - 1) * nx + j] != 0) {
				val += res[(i - 1) * nx + j]; // n
			} else {
				return;
			}
		}
		if (j > 0) {
			if (direction[(i - 1) * nx + (j - 1)] == 5)
				if (res[(i - 1) * nx + (j - 1)] != 0) {
					val += res[(i - 1) * nx + (j - 1)];// no

				} else {
					return;
				}
		}
		if (j < ny - 1) {
			if (direction[(i - 1) * nx + (j + 1)] == 7)
				if (res[(i - 1) * nx + (j + 1)] != 0) {
					val += res[(i - 1) * nx + (j + 1)];// ne

				} else {
					return;
				}
		}
	}
	//SUD
	if (i < nx - 1) {
		if (direction[(i + 1) * nx + j] == 2)
			if (res[(i + 1) * nx + j] != 0) {
				val += res[(i + 1) * nx + j];// s

			} else {
				return;
			}
		if (j > 0) {
			if (direction[(i + 1) * nx + (j - 1)] == 3)
				if (res[(i + 1) * nx + (j - 1)] != 0) {
					val += res[(i + 1) * nx + (j - 1)];// so

				} else {
					return;
				}
		}
		if (j < ny - 1) {
			if (direction[(i + 1) * nx + (j + 1)] == 1)
				if (res[(i + 1) * nx + (j + 1)] != 0) {
					val += res[(i + 1) * nx + (j + 1)];// se

				} else {
					return;
				}
		}
	}
	//Ouest
	if (j > 0) {
		if (direction[index - 1] == 4)
			if (res[index - 1] != 0) {
				val += res[index - 1];// o
			} else {
				return;
			}
	}
	//Est
	if (j < ny - 1) {
		if (direction[index + 1] == 8)
			if (res[index + 1] != 0) {
				val += res[index + 1];// e

			} else {
				return;
			}
	}
	val++;
	res[index] = val;
}