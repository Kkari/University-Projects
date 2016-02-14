/*
We assume a 3x3 (radius: 1) convolution kernel, which is not separable.
Each work-group will process a (TILE_X x TILE_Y) tile of the image.
For coalescing, TILE_X should be multiple of 16.

Instead of examining the image border for each kernel, we recommend to pad the image
to be the multiple of the given tile-size.
*/

//should be multiple of 32 on Fermi and 16 on pre-Fermi...
#define TILE_X 32 

#define TILE_Y 16

// d_Dst is the convolution of d_Src with the kernel c_Kernel
// c_Kernel is assumed to be a float[11] array of the 3x3 convolution constants, one multiplier (for normalization) and an offset (in this order!)
// With & Height are the image dimensions (should be multiple of the tile size)
__kernel __attribute__((reqd_work_group_size(TILE_X, TILE_Y, 1)))
void Convolution(
				__global float* d_Dst,
				__global const float* d_Src,
				__constant float* c_Kernel,
				uint Width,  // Use width to check for image bounds
				uint Height,
				uint Pitch   // Use pitch for offsetting between lines
				)
{
	// OpenCL allows to allocate the local memory from 'inside' the kernel (without using the clSetKernelArg() call)
	// in a similar way to standard C.
	// the size of the local memory necessary for the convolution is the tile size + the halo area
	__local float tile[TILE_Y + 2][TILE_X + 2];

	// TO DO...

	// Fill the halo with zeros

	// Load main filtered area from d_Src

	// Load halo regions from d_Src (edges and corners separately), check for image bounds!

	// Sync threads

	// Perform the convolution and store the convolved signal to d_Dst.

	int LIDx = get_local_id(0);
	int LIDy = get_local_id(1);
	int GIDx = get_global_id(0);
	int GIDy = get_global_id(1);

	//Setting the corners to 0
	if (LIDx == 0 && LIDy == 0) tile[0][0] = 0.f;
	if (LIDx == 0 && LIDy == TILE_Y - 1) tile[TILE_Y + 1][0] = 0.f;
	if (LIDx == TILE_X - 1 && LIDy == 0) tile[0][TILE_X + 1] = 0.f;
	if (LIDx == TILE_X - 1 && LIDy == TILE_Y - 1) tile[TILE_Y + 1][TILE_X + 1] = 0.f;
	
	//Setting the sides to 0
	//Left
	if (LIDx == 0) {
		tile[LIDy + 1][0] = 0.f;
	}
	//Right
	if (LIDx == TILE_X - 1) {
		tile[LIDy + 1][TILE_X + 1] = 0.f;
	}
	//Bottom
	if (LIDy == 0) {
		tile[0][LIDx + 1] = 0.f;
	}
	//Top
	if (LIDy == TILE_Y - 1) {
		tile[TILE_Y + 1][LIDx + 1] = 0.f;
	}

	//Loading main area
	if ((GIDx >= 0 && GIDx < Width) && (GIDy >= 0 && GIDy < Height)) {
		tile[LIDy + 1][LIDx + 1] = d_Src[GIDy * Pitch + GIDx];
	}
	else {
		tile[LIDy + 1][LIDx + 1] = 0.f;
	}

	//Loading halo area
	//Corners
	if (LIDx == 0 && LIDy == 0) {
		if (GIDx > 0 && GIDy > 0) {
			tile[0][0] = d_Src[(GIDy - 1) * Pitch + (GIDx - 1)];
		}
	}
	if (LIDx == 0 && LIDy == TILE_Y - 1) {
		if (GIDx > 0 && GIDy < Height - 1) {
			tile[TILE_Y + 1][0] = d_Src[(GIDy + 1) * Pitch + (GIDx - 1)];
		}
	}
	if (LIDx == TILE_X - 1 && LIDy == 0) {
		if (GIDx < Width - 1 && GIDy > 0) {
			tile[0][TILE_X + 1] = d_Src[(GIDy - 1) * Pitch + (GIDx + 1)];
		}
	}
	if (LIDx == TILE_X - 1 && LIDy == TILE_Y - 1) {
		if (GIDx < Width - 1 && GIDy < Height - 1) {
			tile[TILE_Y + 1][TILE_X + 1] = d_Src[(GIDy + 1) * Pitch + (GIDx + 1)];
		}
	}

	//Left
	if (LIDx == 0 && GIDx > 0) {
		tile[LIDy + 1][0] = d_Src[(GIDy * Pitch + (GIDx - 1))];
	}

	//Right
	if (LIDx == TILE_X - 1 && GIDx < Width - 1) {
		tile[LIDy + 1][TILE_X + 1] = d_Src[(GIDy * Pitch + (GIDx + 1))];
	}

	//Bottom
	if (LIDy == 0 && GIDy > 0) {
		tile[0][LIDx + 1] = d_Src[(GIDy - 1) * Pitch + GIDx];
	}

	//Top
	if (LIDy == TILE_Y - 1 && GIDy < Height - 1) {
		tile[TILE_Y + 1][LIDx + 1] = d_Src[(GIDy + 1) * Pitch + GIDx];
	}

	//Syncing the threads
	barrier(CLK_LOCAL_MEM_FENCE);

	//Convolution
	float pixel = 0.f;
	if ((LIDx >= 0 && LIDx < TILE_X) && (LIDy >= 0 && LIDy < TILE_Y)) {
		for (unsigned int y = 0; y < 3; y++) {
			for (unsigned int x = 0; x < 3; x++) {
				pixel += tile[LIDy + y][LIDx + x] * c_Kernel[y * 3 + x];
			}
		}
		d_Dst[GIDy * Pitch + GIDx] = pixel * c_Kernel[9] + c_Kernel[10];
	}

	
}
