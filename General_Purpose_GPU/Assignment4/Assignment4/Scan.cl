

	//********************************************************
	// GPU Computing only!
	// (If you are doing the smaller course, ignore this file)
	//********************************************************

	// Add your previous parallel prefix sum code here

#define UNROLL
#define NUM_BANKS			32
#define NUM_BANKS_LOG		5
#define SIMD_GROUP_SIZE		32

// Bank conflicts
#define AVOID_BANK_CONFLICTS
#ifdef AVOID_BANK_CONFLICTS
#define OFFSET(A) (A) + (A) / NUM_BANKS
#else
#define OFFSET(A) (A)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Scan(__global uint* array, __global uint* higherLevelArray, __local uint* localBlock)
{
	int LID = get_local_id(0);
	int GID = get_global_id(0);
	unsigned N = get_local_size(0) * 2;
	localBlock[OFFSET(LID * 2)] = array[GID * 2];
	localBlock[OFFSET(LID * 2 + 1)] = array[GID * 2 + 1];
	barrier(CLK_LOCAL_MEM_FENCE);

	for (unsigned stride = 1; stride <= N / 2; stride *= 2)
	{
		if (LID < N / stride / 2)
			localBlock[OFFSET(LID*(stride * 2) + stride * 2 - 1)] += localBlock[OFFSET(LID*(stride * 2) + stride - 1)];//adding elements "next to" each other
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if (LID == 0)
		localBlock[OFFSET(N - 1)] = 0;

	for (unsigned stride = N / 2; stride >= 1; stride /= 2)
	{
		if (LID < N / stride / 2)
		{
			int temp = localBlock[OFFSET(LID*(stride * 2) + stride * 2 - 1)];
			localBlock[OFFSET(LID*(stride * 2) + stride * 2 - 1)] += localBlock[OFFSET(LID*(stride * 2) + stride - 1)];
			localBlock[OFFSET(LID*(stride * 2) + stride - 1)] = temp;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	array[GID * 2] += localBlock[OFFSET(LID * 2)];
	array[GID * 2 + 1] += localBlock[OFFSET(LID * 2 + 1)];

	if (LID == get_local_size(0) - 1)
		higherLevelArray[(GID * 2 + 1) / (get_local_size(0) * 2)] = array[GID * 2 + 1];
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void ScanAdd(__global uint* higherLevelArray, __global uint* array, __local uint* localBlock)
{
	int GID = get_global_id(0);
	if (GID >= get_local_size(0))
	{
		array[2 * GID] += higherLevelArray[2 * GID / (get_local_size(0) * 2) - 1];
		array[2 * GID + 1] += higherLevelArray[(2 * GID + 1) / (get_local_size(0) * 2) - 1];
	}
}

__kernel void ScanNaive(const __global uint* inArray, __global uint* outArray, uint N, uint offset) 
{
	int GID = get_global_id(0);
	if (GID + offset < N)
		outArray[GID + offset] = inArray[GID] + inArray[GID + offset];
	if (GID < offset)
		outArray[GID] = inArray[GID];
}

