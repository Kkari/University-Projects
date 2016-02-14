#define lid_x get_local_id(0)
#define lid_y get_local_id(1)

#define gid_x get_global_id(0)
#define gid_y get_global_id(1)

#define group_x get_group_id(0)
#define group_y get_group_id(1)


//Each thread load exactly one halo pixel
//Thus, we assume that the halo size is not larger than the 
//dimension of the work-group in the direction of the kernel

//to efficiently reduce the memory transfer overhead of the global memory
// (each pixel is lodaded multiple times at high overlaps)
// one work-item will compute RESULT_STEPS pixels

//for unrolling loops, these values have to be known at compile time

/* // These macros will be defined dynamically during building the program

#define KERNEL_RADIUS 2

//horizontal kernel
#define H_GROUPSIZE_X		32
#define H_GROUPSIZE_Y		4
#define H_RESULT_STEPS		2

//vertical kernel
#define V_GROUPSIZE_X		32
#define V_GROUPSIZE_Y		16
#define V_RESULT_STEPS		3

*/

#define KERNEL_LENGTH (2 * KERNEL_RADIUS + 1)


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Horizontal convolution filter

/*
c_Kernel stores 2 * KERNEL_RADIUS + 1 weights, use these during the convolution
*/

//require matching work-group size
__kernel __attribute__((reqd_work_group_size(H_GROUPSIZE_X, H_GROUPSIZE_Y, 1)))
void ConvHorizontal(
			__global float* d_Dst,
			__global const float* d_Src,
			__constant float* c_Kernel,
			int Width,
			int Pitch
			)
{
	//The size of the local memory: one value for each work-item.
	//We even load unused pixels to the halo area, to keep the code and local memory access simple.
	//Since these loads are coalesced, they introduce no overhead, except for slightly redundant local memory allocation.
	//Each work-item loads H_RESULT_STEPS values + 2 halo values
	__local float tile[H_GROUPSIZE_Y][(H_RESULT_STEPS + 2) * H_GROUPSIZE_X];

	// TODO:
	const int baseX = group_x * H_GROUPSIZE_X * H_RESULT_STEPS + lid_x;
	const int baseY = gid_y * Pitch;
	//const int offset = 0; //?

	// Load left halo (check for left bound)

	
	if (group_x != 0)
		tile[lid_y][lid_x] = d_Src[baseY + baseX - H_GROUPSIZE_X];
	else
		tile[lid_y][lid_x] = 0.f;


	// Load main data + right halo (check for right bound)
	for (int tileID = 1; tileID <= H_RESULT_STEPS + 1; ++tileID)
	{
		tile[lid_y][lid_x + tileID * H_GROUPSIZE_X] = d_Src[baseY + baseX + (tileID - 1) * H_GROUPSIZE_X];
	}

	// Sync the work-items after loading

	barrier(CLK_LOCAL_MEM_FENCE);

	// Convolve and store the result
	
	float sum = 0.f;

	for (int i = 1; i <= H_RESULT_STEPS; ++i)
	{
		#pragma unroll
		for (int j = -KERNEL_RADIUS; j <= KERNEL_RADIUS; ++j)
		{
			sum += tile[lid_y][lid_x + i * H_GROUPSIZE_X + j] * c_Kernel[KERNEL_RADIUS - j];
		}
		d_Dst[baseY + baseX + (i - 1) * H_GROUPSIZE_X] = sum;
		sum = 0.f;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertical convolution filter

//require matching work-group size
__kernel __attribute__((reqd_work_group_size(V_GROUPSIZE_X, V_GROUPSIZE_Y, 1)))
void ConvVertical(
			__global float* d_Dst,
			__global const float* d_Src,
			__constant float* c_Kernel,
			int Height,
			int Pitch
			)
{
	__local float tile[(V_RESULT_STEPS + 2) * V_GROUPSIZE_Y][V_GROUPSIZE_X];

	//TO DO:
	// Conceptually similar to ConvHorizontal

	const int baseX = gid_x;
	const int baseY = group_y * V_GROUPSIZE_Y * V_RESULT_STEPS + lid_y;

	// Load top halo + main data + bottom halo

	tile[lid_y][lid_x] = 0.f;
	if (group_y != 0)
		tile[lid_y][lid_x] = d_Src[(baseY - V_GROUPSIZE_Y) * Pitch + baseX];

	for (int tileID = 1; tileID <= V_RESULT_STEPS + 1; ++tileID)
	{
		tile[lid_y + tileID * V_GROUPSIZE_Y][lid_x] = 0.f;
		if (baseY + (tileID - 1) * V_GROUPSIZE_Y < Height)
			tile[lid_y + tileID * V_GROUPSIZE_Y][lid_x] = d_Src[(baseY + (tileID - 1) * V_GROUPSIZE_Y) * Pitch + baseX];
	}

	//Sync the threads

	barrier(CLK_LOCAL_MEM_FENCE);

	// Compute and store results

	float sum = 0.f;

	for (int i = 1; i <= V_RESULT_STEPS; ++i)
	{
		#pragma unroll
		for (int j = -KERNEL_RADIUS; j <= KERNEL_RADIUS; ++j)
		{
			sum += tile[lid_y + i * V_GROUPSIZE_Y + j][lid_x] * c_Kernel[KERNEL_RADIUS - j];
		}
		d_Dst[(baseY + (i - 1) * V_GROUPSIZE_Y) * Pitch + baseX] = sum;
		sum = 0.f;
	}

}
