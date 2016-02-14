#define lid_x get_local_id(0)
#define lid_y get_local_id(1)

#define gid_x get_global_id(0)
#define gid_y get_global_id(1)

#define group_x get_group_id(0)
#define group_y get_group_id(1)

#define lSizeX get_local_size(0)
#define lSizeY get_local_size(1)


#define KERNEL_LENGTH (2 * KERNEL_RADIUS + 1)

#define DEPTH_THRESHOLD	0.025f
#define NORM_THRESHOLD	0.9f

// These functions define discontinuities
bool IsNormalDiscontinuity(float4 n1, float4 n2){
	return fabs(dot(n1, n2)) < NORM_THRESHOLD;
}

bool IsDepthDiscontinuity(float d1, float d2){
	return fabs(d1 - d2) > DEPTH_THRESHOLD;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////
// Horizontal convolution filter

//require matching work-group size
__kernel __attribute__((reqd_work_group_size(H_GROUPSIZE_X, H_GROUPSIZE_Y, 1)))
void DiscontinuityHorizontal(
			__global int* d_Disc,
			__global const float4* d_NormDepth,
			int Width,
			int Height,
			int Pitch
			)
{

	// TODO: Uncomment code and fill in the missing code. 
	// You don't have to follow the provided code. Feel free to adjust it if you want.

	// The size of the local memory: one value for each work-item.
	// We even load unused pixels to the halo area, to keep the code and local memory access simple.
	// Since these loads are coalesced, they introduce no overhead, except for slightly redundant local memory allocation.
	// Each work-item loads H_RESULT_STEPS values + 2 halo values
	// We split the float4 (normal + depth) into an array of float3 and float to avoid bank conflicts.

	__local float tileNormX[H_GROUPSIZE_Y][(H_RESULT_STEPS + 2) * H_GROUPSIZE_X];
	__local float tileNormY[H_GROUPSIZE_Y][(H_RESULT_STEPS + 2) * H_GROUPSIZE_X];
	__local float tileNormZ[H_GROUPSIZE_Y][(H_RESULT_STEPS + 2) * H_GROUPSIZE_X];
	__local float tileDepth[H_GROUPSIZE_Y][(H_RESULT_STEPS + 2) * H_GROUPSIZE_X];

	const int baseX = group_x * H_GROUPSIZE_X * H_RESULT_STEPS + lid_x;
	const int baseY = gid_y * Pitch;
	//const int offset = ...

	//Load left halo (each thread loads exactly one)
	float4 nd = (float4) (1.f, 1.f, 1.f, 0.f);
	
    if (group_x != 0)
    {
		nd = d_NormDepth[baseY + baseX - H_GROUPSIZE_X];
    }

	tileNormX[lid_y][lid_x] = nd.x;
	tileNormY[lid_y][lid_x] = nd.y;
	tileNormZ[lid_y][lid_x] = nd.z;
	tileDepth[lid_y][lid_x] = nd.w;

	// Load main data + right halo
	// pragma unroll is not necessary as the compiler should unroll the short loops by itself.
	#pragma unroll
	for(int i = 1; i <= H_RESULT_STEPS + 1; ++i)
	{
		float4 nd = (float4) (0.f, 0.f, 0.f, 0.f);
		
        if (baseX + (i - 1) * H_GROUPSIZE_X < Width)
        {
			nd = d_NormDepth[baseY + baseX + (i - 1) * H_GROUPSIZE_X];
        }

        tileNormX[lid_y][lid_x + i * H_GROUPSIZE_X] = nd.x;
		tileNormY[lid_y][lid_x + i * H_GROUPSIZE_X] = nd.y;
		tileNormZ[lid_y][lid_x + i * H_GROUPSIZE_X] = nd.z;
		tileDepth[lid_y][lid_x + i * H_GROUPSIZE_X] = nd.w;
	}

	// Sync threads

	barrier(CLK_LOCAL_MEM_FENCE);

	// Identify discontinuities
	#pragma unroll
	for(int i = 0; i <= H_RESULT_STEPS + 1; ++i)
	{
		int flag = 0;

		float myDepth = tileDepth[lid_y][lid_x + i * H_GROUPSIZE_X];
		
        float4 myNorm = (float4) (tileNormX[lid_y][lid_x + i * H_GROUPSIZE_X], 
                                  tileNormY[lid_y][lid_x + i * H_GROUPSIZE_X], 
                                  tileNormZ[lid_y][lid_x + i * H_GROUPSIZE_X], 
                                  0.f);

		// Check the left neighbour
		if (i != 0 || lid_x != 0)
		{
			float leftDepth = tileDepth[lid_y][lid_x + i * H_GROUPSIZE_X - 1];
			float4 leftNorm = (float4) (tileNormX[lid_y][lid_x + i * H_GROUPSIZE_X - 1], 
                                        tileNormY[lid_y][lid_x + i * H_GROUPSIZE_X - 1], 
                                        tileNormZ[lid_y][lid_x + i * H_GROUPSIZE_X - 1], 
                                        0.f);
			
			if (IsDepthDiscontinuity(myDepth, leftDepth) || IsNormalDiscontinuity(myNorm, leftNorm))
            {
				flag |= 1;
            }
		}
		
        // Check the right neighbour
		if (i != H_RESULT_STEPS + 1 || lid_x < lSizeX - 1)
		{
			float rightDepth = tileDepth[lid_y][lid_x + i * H_GROUPSIZE_X + 1];
			float4 rightNorm = (float4) (tileNormX[lid_y][lid_x + i * H_GROUPSIZE_X + 1], 
                                         tileNormY[lid_y][lid_x + i * H_GROUPSIZE_X + 1], 
                                         tileNormZ[lid_y][lid_x + i * H_GROUPSIZE_X + 1], 
                                         0.f);
			
			if (IsDepthDiscontinuity(myDepth, rightDepth) || IsNormalDiscontinuity(myNorm, rightNorm))
            {
                flag |= 2;
            }
		}

		// Write the flag out
		d_Disc[baseY + baseX + (i - 1) * H_GROUPSIZE_X] = flag;
	}	

	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertical convolution filter

//require matching work-group size
__kernel __attribute__((reqd_work_group_size(V_GROUPSIZE_X, V_GROUPSIZE_Y, 1)))
void DiscontinuityVertical(
			__global int* d_Disc,
			__global const float4* d_NormDepth,
			int Width,
			int Height,
			int Pitch
			)
{
	// Comments in the DiscontinuityHorizontal should be enough.
	// TODO

	// WARNING: For profiling reasons, it might happen that the framework will run
	// this kernel several times.

	// You need to make sure that the output of this kernel DOES NOT influence the input.
	// In this case, we are both reading and writing the d_Disc[] buffer.

	// here is a proposed solution: use separate flags for the vertical discontinuity
	// and merge this with the global discontinuity buffer, using bitwise OR.
	// This way do do not depent on the number of kernel executions.

	__local float tileNormX[(V_RESULT_STEPS + 2) * V_GROUPSIZE_Y][V_GROUPSIZE_X];
	__local float tileNormY[(V_RESULT_STEPS + 2) * V_GROUPSIZE_Y][V_GROUPSIZE_X];
	__local float tileNormZ[(V_RESULT_STEPS + 2) * V_GROUPSIZE_Y][V_GROUPSIZE_X];
	__local float tileDepth[(V_RESULT_STEPS + 2) * V_GROUPSIZE_Y][V_GROUPSIZE_X];

	const int baseX = gid_x;
	const int baseY = group_y * V_GROUPSIZE_Y * V_RESULT_STEPS + lid_y;

	//Load right halo (each thread loads exactly one)
	float4 nd = (float4) (1.f, 1.f, 1.f, 0.f);
	
    if (group_y != 0)
    {
        nd = d_NormDepth[(baseY - V_GROUPSIZE_Y) * Pitch + baseX];
    }

	tileNormX[lid_y][lid_x] = nd.x;
	tileNormY[lid_y][lid_x] = nd.y;
	tileNormZ[lid_y][lid_x] = nd.z;
	tileDepth[lid_y][lid_x] = nd.w;

	// Load main data + bottom halo
	// pragma unroll is not necessary as the compiler should unroll the short loops by itself.
	#pragma unroll
	for(int i = 1; i <= V_RESULT_STEPS + 1; ++i)
	{
		
        float4 nd = (float4) (0.f, 0.f, 0.f, 0.f);
		
        if (baseY + (i - 1) * V_GROUPSIZE_Y < Height)
        {
            nd = d_NormDepth[(baseY + (i - 1) * V_GROUPSIZE_Y) * Pitch + baseX];
        }
		
        tileNormX[lid_y + i * V_GROUPSIZE_Y][lid_x] = nd.x;
        tileNormY[lid_y + i * V_GROUPSIZE_Y][lid_x] = nd.y;
		tileNormZ[lid_y + i * V_GROUPSIZE_Y][lid_x] = nd.z;
		tileDepth[lid_y + i * V_GROUPSIZE_Y][lid_x] = nd.w;
	}

	// Sync threads
	
	barrier(CLK_LOCAL_MEM_FENCE);

	// Identify discontinuities
	#pragma unroll
	for(int i = 0; i <= V_RESULT_STEPS + 1; ++i)
	{
		int flag = 0;

		float myDepth = tileDepth[lid_y + i * V_GROUPSIZE_Y][lid_x];
		float4 myNorm = (float4) (tileNormX[lid_y + i * V_GROUPSIZE_Y][lid_x], 
                                  tileNormY[lid_y + i * V_GROUPSIZE_Y][lid_x], 
                                  tileNormZ[lid_y + i * V_GROUPSIZE_Y][lid_x], 
                                  0.f);

		// Check the top neighbor
		if (i != 0 || lid_y != 0)
		{
			float topDepth = tileDepth[lid_y + i * V_GROUPSIZE_Y - 1][lid_x];
			
            float4 topNorm = (float4) (tileNormX[lid_y + i * V_GROUPSIZE_Y - 1][lid_x], 
                                       tileNormY[lid_y + i * V_GROUPSIZE_Y - 1][lid_x], 
                                       tileNormZ[lid_y + i * V_GROUPSIZE_Y - 1][lid_x], 
                                       0.f);
			
			if (IsDepthDiscontinuity(myDepth, topDepth) || IsNormalDiscontinuity(myNorm, topNorm))
            {
				flag |= 4;
            }
		}

		// Check the bottom neighbor
		if (i != V_RESULT_STEPS + 1 || lid_y != lSizeY - 1)
		{
			float bottomDepth = tileDepth[lid_y + i * V_GROUPSIZE_Y + 1][lid_x];
			
            float4 bottomNorm = (float4) (tileNormX[lid_y + i * V_GROUPSIZE_Y + 1][lid_x], 
                                          tileNormY[lid_y + i * V_GROUPSIZE_Y + 1][lid_x], 
                                          tileNormZ[lid_y + i * V_GROUPSIZE_Y + 1][lid_x], 
                                          0.f);
			
			if (IsDepthDiscontinuity(myDepth, bottomDepth) || IsNormalDiscontinuity(myNorm, bottomNorm))
            {
				flag |= 8;
            }
		}
		
        // Write the flag out
		d_Disc[(baseY + (i - 1) * V_GROUPSIZE_Y) * Pitch + baseX] |= flag; // do NOT use '='
	}
}









//////////////////////////////////////////////////////////////////////////////////////////////////////
// Horizontal convolution filter

//require matching work-group size
__kernel __attribute__((reqd_work_group_size(H_GROUPSIZE_X, H_GROUPSIZE_Y, 1)))
void ConvHorizontal(
			__global float* d_Dst,
			__global const float* d_Src,
			__global const int* d_Disc,
			__constant float* c_Kernel,
			int Width,
			int Height,
			int Pitch
			)
{

	// TODO
	// This will be very similar to the separable convolution, except that you have
	// also load the discontinuity buffer into the local memory
	// Each work-item loads H_RESULT_STEPS values + 2 halo values
	__local float tile[H_GROUPSIZE_Y][(H_RESULT_STEPS + 2) * H_GROUPSIZE_X];
	__local int   disc[H_GROUPSIZE_Y][(H_RESULT_STEPS + 2) * H_GROUPSIZE_X];

	const int baseX = group_x * H_GROUPSIZE_X * H_RESULT_STEPS + lid_x;
	const int baseY = gid_y * Pitch;

	// Load data to the tile and disc local arrays

	tile[lid_y][lid_x] = 0.f;
	disc[lid_y][lid_x] = 15; //0b1111
	
    if (group_x != 0)
	{
		tile[lid_y][lid_x] = d_Src[baseY + baseX - H_GROUPSIZE_X];
		disc[lid_y][lid_x] = d_Disc[baseY + baseX - H_GROUPSIZE_X];
	}

	for (int i = 1; i <= H_RESULT_STEPS + 1; ++i)
	{
		tile[lid_y][lid_x + i * H_GROUPSIZE_X] = 0.f;
		disc[lid_y][lid_x + i * H_GROUPSIZE_X] = 15; //0b1111
		
        if (baseX + (i - 1) * H_GROUPSIZE_X < Width)
		{
			tile[lid_y][lid_x + i * H_GROUPSIZE_X] = d_Src[baseY + baseX + (i - 1) * H_GROUPSIZE_X];
			disc[lid_y][lid_x + i * H_GROUPSIZE_X] = d_Disc[baseY + baseX + (i - 1) * H_GROUPSIZE_X];
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	// During the convolution iterate inside-out from the center pixel towards the borders.
	for (int i = 1; i <= H_RESULT_STEPS; ++i) // Iterate over tiles
	{
		float sum = tile[lid_y][lid_x + i * H_GROUPSIZE_X] * c_Kernel[KERNEL_RADIUS];
		float sumWeight = c_Kernel[KERNEL_RADIUS];
		
		// When you iterate to the left, check for 'left' discontinuities. 
		for (int j = -1; j >= -KERNEL_RADIUS; --j)
		{
			// If you find relevant discontinuity, stop iterating
			if (disc[lid_y][lid_x + i * H_GROUPSIZE_X + (j + 1)] & 1)
            {
				break;
            }
			
            sum += tile[lid_y][lid_x + i * H_GROUPSIZE_X + j] * c_Kernel[KERNEL_RADIUS - j];
			sumWeight += c_Kernel[j + KERNEL_RADIUS];
		}

		// When iterating to the right, check for 'right' discontinuities.
		for (int j = 1; j <= KERNEL_RADIUS; ++j)
		{
			// If you find a relevant discontinuity, stop iterating
			if (disc[lid_y][lid_x + i * H_GROUPSIZE_X + (j - 1)] & 2)
            {
                break;
            }

			sum += tile[lid_y][lid_x + i * H_GROUPSIZE_X + j] * c_Kernel[KERNEL_RADIUS - j];
			sumWeight += c_Kernel[j + KERNEL_RADIUS];
		}

		// Don't forget to accumulate the weights to normalize the kernel (divide the pixel value by the summed weights)
		sum /= sumWeight;
		
		d_Dst[baseY + baseX + (i - 1) * H_GROUPSIZE_X] = sum;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertical convolution filter



//require matching work-group size
__kernel __attribute__((reqd_work_group_size(V_GROUPSIZE_X, V_GROUPSIZE_Y, 1)))
void ConvVertical(
			__global float* d_Dst,
			__global const float* d_Src,
			__global const int* d_Disc,
			__constant float* c_Kernel,
			int Width,
			int Height,
			int Pitch
			)
{
	
	__local float tile[(V_RESULT_STEPS + 2) * V_GROUPSIZE_Y][V_GROUPSIZE_X];
	__local int disc[(V_RESULT_STEPS + 2) * V_GROUPSIZE_Y][V_GROUPSIZE_X];

	const int baseX = gid_x;
	const int baseY = group_y * V_GROUPSIZE_Y * V_RESULT_STEPS + lid_y;

	tile[lid_y][lid_x] = 0.f;
	disc[lid_y][lid_x] = 15; //0b1111
	
    if (group_y != 0)
	{
		tile[lid_y][lid_x] = d_Src[(baseY - V_GROUPSIZE_Y) * Pitch + baseX];
		disc[lid_y][lid_x] = d_Disc[(baseY - V_GROUPSIZE_Y) * Pitch + baseX];
	}

	for (int tileID = 1; tileID <= V_RESULT_STEPS + 1; ++tileID)
	{
		tile[lid_y + tileID * V_GROUPSIZE_Y][lid_x] = 0.f;
		disc[lid_y + tileID * V_GROUPSIZE_Y][lid_x] = 15; //0b1111
		
        if (baseY + (tileID - 1) * V_GROUPSIZE_Y < Height)
		{
			tile[lid_y + tileID * V_GROUPSIZE_Y][lid_x] = d_Src[(baseY + (tileID - 1) * V_GROUPSIZE_Y) * Pitch + baseX];
			disc[lid_y + tileID * V_GROUPSIZE_Y][lid_x] = d_Disc[(baseY + (tileID - 1) * V_GROUPSIZE_Y) * Pitch + baseX];
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	for (int i = 1; i <= V_RESULT_STEPS; ++i) // Iterate over tiles
	{
		float sum = tile[lid_y + i * V_GROUPSIZE_Y][lid_x] * c_Kernel[KERNEL_RADIUS];
		float sumWeight = c_Kernel[KERNEL_RADIUS];
		
		// When you iterate to the top, check for 'top' discontinuities. 
		for (int j = -1; j >= -KERNEL_RADIUS; --j)
		{
			// If you find relevant discontinuity, stop iterating
			if (disc[lid_y + i * V_GROUPSIZE_Y + (j + 1)][lid_x] & 4)
            {
                break;
            }

			sum += tile[lid_y + i * V_GROUPSIZE_Y + j][lid_x] * c_Kernel[KERNEL_RADIUS - j];
			sumWeight += c_Kernel[j + KERNEL_RADIUS];
		}

		// When iterating to the bottom, check for 'bottom' discontinuities.
		for (int j = 1; j <= KERNEL_RADIUS; ++j)
		{
			// If you find a relevant discontinuity, stop iterating
			if (disc[lid_y + i * V_GROUPSIZE_Y + (j - 1)][lid_x] & 8)
            {
                break;
            }

			sum += tile[lid_y + i * V_GROUPSIZE_Y + j][lid_x] * c_Kernel[KERNEL_RADIUS - j];
			sumWeight += c_Kernel[j + KERNEL_RADIUS];
		}

		// Don't forget to accumulate the weights to normalize the kernel (divide the pixel value by the summed weights)
		sum /= sumWeight;
		
		d_Dst[(baseY + (i - 1) * V_GROUPSIZE_Y) * Pitch + baseX] = sum;
	}
}




