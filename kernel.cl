__kernel
void directions(const int sz_x, const int sz_y, const int nodata, __global const float *const data, __global unsigned char *const directions) {
	__private const int x = get_global_id(0);
	__private const int y = get_global_id(1);

	const unsigned long max_x = sz_x - 1;
	const unsigned long max_y = sz_y - 1;

	//if (((int) get(data, x, y, sz_x)) != nodata) {
	if (((int) data[x * sz_x + y]) != nodata) {

		//float minimum_value = get(data, x, y, sz_x);
		float minimum_value = data[x * sz_x + y];
		float current;
		unsigned char direction = 0;

		if (x > 0 && y > 0) {
			//current = get(data, x - 1, y - 1, sz_x);
			current = data[(x - 1) * sz_x + y - 1];
			if (current < minimum_value && ((int) current != nodata)) {
				direction = 1;
				minimum_value = current;
			}
		}

		if (x > 0) {
			//current = get(data, x - 1, y, sz_x);
			current = data[(x - 1) * sz_x + y];
			if (current < minimum_value && ((int) current != nodata)) {
				direction = 2;
				minimum_value = current;
			}
		}

		if (x > 0 && y < max_y) {
			//current = get(data, x - 1, y + 1, sz_x);
			current = data[(x - 1) * sz_x + y + 1];
			if (current < minimum_value && ((int) current != nodata)) {
				direction = 3;
				minimum_value = current;
			}
		}

		if (y < max_y) {
			current = data[x * sz_x + y + 1];
			if (current < minimum_value && ((int) current != nodata)) {
				direction = 4;
				minimum_value = current;
			}
		}

		if (y < max_y && x < max_x) {
			//current = get(data, x + 1, y + 1, sz_x);
			current = data[(x + 1) * sz_x + y + 1];
			if (current < minimum_value && ((int) current != nodata)) {
				direction = 5;
				minimum_value = current;
			}
		}

		if (x < max_x) {
			current = data[(x + 1) * sz_x + y];
			if (current < minimum_value && ((int) current != nodata)) {
				direction = 6;
				minimum_value = current;
			}
		}

		if (x < max_x && y > 0) {
			//current = get(data, x + 1, y - 1, sz_x);
			current = data[(x + 1) * sz_x + y - 1];
			if (current < minimum_value && ((int) current != nodata)) {
				direction = 7;
				minimum_value = current;
			}
		}

		if (y > 0) {
			current = data[x * sz_x + y - 1];
			if (current < minimum_value && ((int) current != nodata)) direction = 8;
		}

		directions[x * sz_x + y] = direction;
	}
}

__kernel
void compute(const int sz_x, const int sz_y, __global const unsigned char *const directions, __global unsigned char *const water, __global bool* const hasChanged) {
	__private const int x = get_global_id(0);
	__private const int y = get_global_id(1);

	const unsigned long max_x = sz_x - 1;
	const unsigned long max_y = sz_y - 1;
}