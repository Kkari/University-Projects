
// Rotate the matrix CLOCKWISE

//naive implementation: move the elements of the matrix directly to their destinations
//this will cause unaligned memory accessed which - as we will see - should be avoided on the GPU

__kernel void MatrixRotNaive(__global const float* M, __global float* MR, uint SizeX, uint SizeY)
{
	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);
	
	if(GID.x < SizeX && GID.y < SizeY) {
	  MR[GID.x * SizeY + (SizeY - GID.y - 1)] = M[GID.y * SizeX + GID.x];
	}
}

//this kernel does the same thing, however, the local memory is used to
//transform a small chunk of the matrix locally
//then write it back after synchronization in a coalesced access pattern
__kernel void MatrixRotOptimized(__global const float* M, __global float* MR, uint SizeX, uint SizeY,
							__local float* block)
{
	//// TO DO: Add kernel code
	int2 GID;
	GID.x = get_global_id(0);
	GID.y = get_global_id(1);

	int2 LID;
	LID.x = get_local_id(0);
	LID.y = get_local_id(1);
	
	// Reading into local memory.
	if (GID.x < SizeX && GID.y < SizeY)
	{
	    // Coalesced pattern
	    block[LID.y * get_local_size(0) + LID.x] = M[GID.y * SizeX + GID.x];
	}
	
	// The new local ID.
	// It works so that NLID.x makes up the new y axis which takes the absolut placement of the thread
	// in the block into accout.
	// NLID.y will make up the new x axis from left to right.
	// For example for (0,1) in a block of 2x3
	// NLID.x = ((1 * 2 + 0 )/ 3) = 0  and NLID.y = 3 - 1 - (2) = 0
	// One has to note that, if there are more threads for the block than valid elements, then it will
	// overindex, so into negative values and that will wreak havoc. That is why we have to check it again.
	int2 NLID;
	NLID.x = (LID.y * get_local_size(0) + LID.x) / get_local_size(1);
	NLID.y = get_local_size(1) - 1 - ((LID.y * get_local_size(0) + LID.x) % get_local_size(1)); 
	
	// The new global ID.
	// Same block, but different place inside the block.
	// As the NLID transposes and reverses the block in itself it will just make new GID out of the old ones.
	// For example at the 2x3 block now the NGID of the first 3 elements will be 0 and not 0,0 and then 1.
	// Same kind of rearrangement happens also at y.
	// Example:
	// 1 2
	// 3 4      5 3 1
	// 5 6 ---> 6 4 2
	// For (0,1) will NGID be (0, 0)
	// For (1,1) will NGID be (1, 2) 
	int2 NGID;
	NGID.x = GID.x - LID.x + NLID.x; 
	NGID.y = GID.y - LID.y + NLID.y;
	
	barrier(CLK_LOCAL_MEM_FENCE);
	
	// Over-indexing is checked on the new IDs
	if (NGID.x < SizeX && NGID.y < SizeY)
	{
	    // Same rotation algorithm as before, but using the new IDs
	    // It will be coalesced, because the row jumps happen with NGID.x, which is our new y axis.
	    // And it only hops SizeY memory brackets when our row full is, on the other hand NGID.y
	    // fills the lines up. NGID.y is our new x axis, that ranges from 0 to 2 in my little example.
	    // The reverse placement of the blocks happens at (SizeY - NGID.y -1). 
	    MR[NGID.x * SizeY + (SizeY - NGID.y - 1)] = block[NLID.y * get_local_size(0) + NLID.x];
	}
}


/* Author: Zoltan Bogaromi
__kernel void MatrixRotOptimized(__global const float* M, __global float* MR, uint SizeX, uint SizeY, __local float* block)
{
        // TO DO: Add kernel code
        uint2 GID;
        GID.x = get_global_id(0);
        GID.y = get_global_id(1);
        uint2 LID;
        LID.x = get_local_id(0);
        LID.y = get_local_id(1);
       
        if (GID.x < SizeX && GID.y < SizeY)
        {
               
                uint effectiveLocalSizeY = get_local_size(1); //this variable stores the real Y dimension of the tile (important, if work item IDs "point out")
                int2 O; //stores the global (x,y) indices of the tile element (index applies to rotated matrix), which has local indices (0,0)
                        //this determines the position of the tile in the rotated matrix
                O.y = GID.x - LID.x;
                O.x = SizeY - GID.y - effectiveLocalSizeY + LID.y; //O.x = (SizeY - 1 - GID.y) - (effectiveLocalSizeY - 1 - LID.y);
                if (O.x < 0) //2DRange is bigger than matrix
                {
                        effectiveLocalSizeY += O.x;
                        O.x = 0;
                }
                uint serial = LID.y * get_local_size(0) + LID.x; //ID of the work item, horizontally neighbouring work items have adjacent serial numbers
                uint2 newLocalPos; //stores the relative position of tile element, which the actual work item has to write, to O
                                   //this determines the place of element in the rotated tile
                newLocalPos.y = serial / effectiveLocalSizeY;
                newLocalPos.x = serial % effectiveLocalSizeY;
 
                block[serial] = M[GID.y * SizeX + GID.x];
 
                //we need to wait for other local threads to finish writing this shared array
                barrier(CLK_LOCAL_MEM_FENCE);
 
                //global position of tile (O) + local position of tile element (newLocalSize) = global position of tile element
                //as newLocalPos is applied to rotated matrix, it can not be applied directly to "block", it has to be rotated back (counter clockwise)
                MR[(O.y + newLocalPos.y) * SizeY + (O.x + newLocalPos.x)] = block[(effectiveLocalSizeY - 1 - newLocalPos.x) * get_local_size(0) + newLocalPos.y];
        }
}
*/
