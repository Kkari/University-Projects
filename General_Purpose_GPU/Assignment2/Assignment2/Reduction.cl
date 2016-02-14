
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Reduction_InterleavedAddressing(__global uint* array, uint stride) 
{
	// TO DO: Kernel implementation
    uint index = (get_global_id(0) << 1) << stride;
    uint size = get_global_size(0);
    uint id = get_global_id(0);
//   printf("\n g_s: %u stride: %u gid: %u f: %u sec: %u", size, stride, id, index, index + stride);
    array[index] += array[index + (1 << stride)];
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Reduction_SequentialAddressing(__global uint* array, uint stride) 
{
	// TO DO: Kernel implementation
//    printf("\n i: %u stride: %u", index, stride);
    
    array[get_global_id(0)] += array[get_global_id(0) + stride];
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Reduction_Decomp(const __global uint* inArray, __global uint* outArray, uint N, __local uint* localBlock)
{
    uint shift = get_global_size(0);
    uint lid = get_local_id(0);
    uint gid = get_global_id(0);

    // if (get_local_id(0) == 0) printf("\n\nHey I'm a kernel, shift: %u", shift);
    // Time we 
	localBlock[get_local_id(0)] = inArray[get_global_id(0) + shift] + inArray[get_global_id(0)];
    //  printf("\nReading in: %d %d",  inArray[get_global_id(0)], inArray[get_global_id(0) + shift]);
    barrier(CLK_LOCAL_MEM_FENCE);

    // It could be better to work with half as many threads from an optimization point of view
    uint active_thread_num = get_local_size(0) / 2;

    //  N is the number of needed iterations, which is log2(local_size(0)) in this case
    for(uint i = 0; i < N; i++) {
        // only let the thread run if it has to in this reduction
        if (get_local_id(0) < active_thread_num) 
        {
            // printf("\n LID: %u, GID: %u, i: %u", lid, gid, i);

            // Basically get the thread_id * 2 * pow(2, i), so it will always grab
            // the element on a 2 aligned place, that is good for us.
            // pl -> tid(0) -> 0, 0, 0; tid(1) -> 2, 4, 16; tid(2) -> 4, 8, 32
            uint index = (get_local_id(0) << 1) << i;
            
            // printf("\n index1: %d", index);
            // printf("\n Adding: %d %d, ", localBlock[index], localBlock[index + (1 << i)]);
            // Grab the index place, and the add the stride to it, that is a power of 2.
            // -> 1, 2, 4, 16... etc
            localBlock[index] += localBlock[index + (1 << i)];
        }
        // printf("\nInner printf:\n  %d %d", active_thread_num, localBlock[0]); 
        
        // In the next iteration we will only need half of the threads
        active_thread_num /= 2;

        // We must wait here because if there are more than a warp active threads, then
        // race condition can occur in this loop.
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    // if(get_local_id(0) == 0) printf("\nmy result is: %d", localBlock[0]);
    // if(get_local_id(0) == 0) printf("\nmy result is: %d", localBlock[0]);
    // If we are done with the reduction we must simply write it back to the array.
    outArray[get_group_id(0)] = localBlock[0];
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Reduction_DecompUnroll(const __global uint* inArray, __global uint* outArray, uint N, __local uint* localBlock)
{

        uint shift = get_global_size(0);
        uint lid = get_local_id(0);
        uint gid = get_global_id(0);
        uint active_thread_num = get_local_size(0) / 2;

        //if (get_global_id(0) == 0) printf("\n\nHey I'm a kernel, shift: %u", shift);
        // Time we 
        localBlock[get_local_id(0)] = inArray[get_global_id(0) + shift] + inArray[get_global_id(0)];
        // printf("\nReading in: %d %d",  inArray[get_global_id(0)], inArray[get_global_id(0) + shift]);
        barrier(CLK_LOCAL_MEM_FENCE);

        if (active_thread_num >= 32) {
            //if (get_local_id(0) == 0) printf("\n number of active threads at first: %d", active_thread_num);

            //if (get_global_id(0) == 0) printf("\n number of active threads: %d", active_thread_num);
            //  N is the number of needed iterations, which is log2(local_size(0)) in this case
            uint i = 0;
            for(i = 0; active_thread_num > 32; i++) {
                if (get_local_id(0) < active_thread_num) 
                {
                    // printf("\n LID: %u, GID: %u, i: %u", lid, gid, i);
                    uint index = (get_local_id(0) << 1) << i;
                    // printf("\n Adding: %d %d, ", localBlock[index], localBlock[index + (1 << i)]);

                    localBlock[index] += localBlock[index + (1 << i)];
                }
                // printf("\nInner printf:\n  %d %d", active_thread_num, localBlock[0]); 
                // if (get_local_id(0) == 0) printf("\n number of active threads: %d", active_thread_num);
                active_thread_num /= 2;
                // if(get_global_id(0) == 0) printf("\nin da loop");
                barrier(CLK_LOCAL_MEM_FENCE);
            }

           // if(get_local_id(0) == 0) printf("\nmy result is: %d", localBlock[0]);
        //    if (get_global_id(0)  == 0) printf("\nthe index is: %d", i);
            //  if (get_global_id(0) == 0) printf("\n number of active threads: %d", active_thread_num);
        // if (get_global_id(0) == 0) printf("\n displacement power: %d", i);
            
            // If the shift is 1, then we don't need this loop, because that means that
            // we are at the last kernel, that has only 2 elements and it will just simply
            // add them together. The get_local_id(0) < 32 is needed, so other warps can't mess
            // up this addition by scheduleing
            if (get_local_id(0) < 32 && shift != 1) {
                uint temp = localBlock[0];

                #pragma unroll
                for(uint j = 0; j < 32; j++) {
                    uint index = (j << 1) << i;
            //      if (get_global_id(0) == 0) printf("\n index: %d", index);
                    localBlock[0] += localBlock[index];
                    localBlock[0] += localBlock[index + (1 << i)];
                }
                
                localBlock[0] -= temp;
            //    if(get_local_id(0) == 0) printf("\nmy result is: %d", localBlock[0]);
            }
        }
        else 
        {
            // printf("\nI am here in the else branch");
            for(uint i = 1; i < get_global_size(0); i++) 
            {
                // if(get_global_id(0) == 0) printf("\nAdding %d", localBlock[i]);
                localBlock[0] += localBlock[i];
            }

        //    if(get_local_id(0) == 0) printf("\nmy result is: %d", localBlock[0]);
        }
    barrier(CLK_LOCAL_MEM_FENCE);
    outArray[get_group_id(0)] = localBlock[0];
}
