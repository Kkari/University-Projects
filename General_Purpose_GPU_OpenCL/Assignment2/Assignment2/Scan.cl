//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Scan_Naive(const __global uint* inArray, __global uint* outArray, uint N, uint offset) 
{
	// TO DO: Kernel implementation
    
    if (get_global_id(0) < N) {
        uint index = get_global_id(0);
        //printf("\n el1: %d, el2: %d", index, index + offset);

        outArray[get_global_id(0) + offset] = inArray[get_global_id(0) + offset] + inArray[get_global_id(0)];

   /*     printf("\nAdding: %d and %d to position %d, result: %d", 
                inArray[get_global_id(0) + offset],
                inArray[index],
                index + offset,
                outArray[get_global_id(0) + offset]);
                */

        // copy the elements that hasn't been copied yet, but considered
        // done. Like the 0. element in the first round, 1 in second etc.
        // it could be optimized with coalescing
        if (get_global_id(0) < offset && get_global_id(0) >= (offset >> 1)) 
        {
            outArray[get_global_id(0)] = inArray[get_global_id(0)];
        }
    }
 //   if(get_global_id(0) == 0) printf("\n\n"); 
}



// Why did we not have conflicts in the Reduction? Because of the sequential addressing (here we use interleaved => we have conflicts).

#define UNROLL
#define NUM_BANKS			32
#define NUM_BANKS_LOG		5
#define SIMD_GROUP_SIZE		32

// Bank conflicts
#define AVOID_BANK_CONFLICTS
#ifdef AVOID_BANK_CONFLICTS
	// TO DO: define your conflict-free macro here
    #define OFFSET(A) ((A) + (A >> 5))
#else
	#define OFFSET(A) (A)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Scan_WorkEfficient(__global uint* array, __global uint* higherLevelArray, __local uint* localBlock)
{
	// TO DO: Kernel implementation

	//uint GID = get_global_id(0);
	//uint LID = get_local_id(0);
	uint GID_data = get_global_id(0) << 1;
	uint LID_data = get_local_id(0) << 1;
	//uint LSize = get_local_size(0);
	uint LSize_data = get_local_size(0) << 1;

	// get the elements in a striped manner, this could be improved with coalescing... but... 
    localBlock[OFFSET(LID_data)] = array[GID_data];
    
    // we will make the first addition here, so we will start the strides at 4
	localBlock[OFFSET(LID_data + 1)] = array[GID_data] + array[GID_data + 1];
	barrier(CLK_LOCAL_MEM_FENCE);

    // up-sweep
	//maxLID: above were all active, now, only half of them
	for (uint stride = 4, maxLID = get_local_size(0) / 2; stride <= LSize_data; stride *= 2, maxLID /= 2)
	{
        // making a strided access so that it will reach the las element in the end
        // it works so for example that lid:0 -> lb[4] += lb[2]
		if (get_local_id(0) < maxLID)
			localBlock[OFFSET(get_local_id(0) * stride + (stride - 1))] += localBlock[OFFSET(get_local_id(0) * stride + ((stride / 2) - 1))];
		barrier(CLK_LOCAL_MEM_FENCE);
	}

    // let's set 0 for the last element
	if (get_local_id(0) == 0)
		localBlock[OFFSET(LSize_data - 1)] = 0;
	
	barrier(CLK_LOCAL_MEM_FENCE);

	// down-sweep
    for (uint stride = get_local_size(0), maxLID = 1; stride > 1; stride >>= 1, maxLID <<= 1)
	{
		if (get_local_id(0) < maxLID)
		{
			uint left_index = (LID_data + 1) * stride - 1; 
			uint right_index = (LID_data + 2) * stride - 1; 

            // we store the left child, so it won't be lost when we overwrite it
			uint left_temp = localBlock[OFFSET(left_index)]; 

            // let's overwrite it with the right child
			localBlock[OFFSET(left_index)] = localBlock[OFFSET(right_index)]; 

            // adding them together
			localBlock[OFFSET(right_index)] += left_temp;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

    // update the array with the partials
	array[GID_data] += localBlock[OFFSET(LID_data + 1)];
	array[GID_data + 1] += localBlock[OFFSET(LID_data)] + localBlock[OFFSET(LID_data + 1)];

    // get the last element and write it in the other array
	if (get_local_id(0) == get_local_size(0) - 1)
	{
		higherLevelArray[get_group_id(0)] = array[GID_data + 1];
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Scan_WorkEfficientAdd(__global uint* higherLevelArray, __global uint* array, __local uint* localBlock)
{
	// TO DO: Kernel implementation (large arrays)
	// Kernel that should add the group PPS to the local PPS (Figure 14)
	
	// merging the partials sums
    // for the first group there is no need to merge anything, so we let that rest in peace
    if (get_group_id(0) > 0)
	{
        // Over hours experimented black magic indexing
		uint index = ((get_global_id(0) - get_local_id(0)) * 2) + get_local_id(0);
        uint term = higherLevelArray[get_group_id(0) - 1];
		array[index] += term;
		array[index + get_local_size(0)] += term;
	}
}

