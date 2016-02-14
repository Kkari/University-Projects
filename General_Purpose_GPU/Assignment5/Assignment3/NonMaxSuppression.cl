#define lid_x get_local_id(0)
#define lid_y get_local_id(1)
#define gid_x get_global_id(0)
#define gid_y get_global_id(1)
#define lSize_x get_local_size(0)
#define lSize_y get_local_size(1)

/*
Each work-group will process a (TILE_X x TILE_Y) tile of the image.
For coalescing, TILE_X should be multiple of 16.

Instead of examining the image border for each kernel, we recommend to pad the image
to be the multiple of the given tile-size.
*/

//should be multiple of 32 on Fermi and 16 on pre-Fermi...
#define TILE_X 32 
#define TILE_Y 16

// With & Height are the image dimensions (should be multiple of the tile size)
__kernel __attribute__((reqd_work_group_size(TILE_X, TILE_Y, 1)))
void NonMaxSuppression(
				__global float* d_Dst,
				__global const uchar* theta,
				__global const float* d_Src,
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
  
    tile[lid_y + 2][lid_x + 2] = 0.f;
    tile[lid_y][lid_x] = 0.f;
    tile[lid_y + 2][lid_x] = 0.f;
    tile[lid_y][lid_x + 2] = 0.f;

    // There can be a race condition here if some threads progress before completly filling the Halo up with zeroes
    
    barrier(CLK_LOCAL_MEM_FENCE);
    //tile[lid_y + 1][lid_x + 1] = d_Src[gid_y * Pitch + gid_x];
	
	if ((gid_x >= 0 && gid_x < Width) && (gid_y >= 0 && gid_y < Height)) {
		tile[lid_y + 1][lid_x + 1] = d_Src[gid_y * Pitch + gid_x];
	}
	else {
		tile[lid_y + 1][lid_x + 1] = 0.f;
	}
    // Load halo regions from d_Src (edges and corners separately), check for image bounds!
    
    // If it is the first column in the kernel, and it is not the first column in the whole image, then load
    // gid_x - 1 > 0 will not be good, because these are unsigned ints and at 0 it overflows.
    if ((gid_x > 0) && (lid_x == 0))
    {
        tile[lid_y + 1][0] = d_Src[gid_y * Pitch + (gid_x - 1)];
        
        // Upper-left corner (Element [0][0])
        if ((gid_y > 0) && (lid_y == 0)) 
        {
            tile[0][0] = d_Src[(gid_y - 1) * Pitch + (gid_x - 1)];
        }

        // Lower-left corner (Element [TILE_Y + 1][0])
        if ((gid_y < (Height - 1)) && (lid_y == (TILE_Y - 1)))
        {
            tile[TILE_Y + 1][0] = d_Src[(gid_y + 1) * Pitch + (gid_x - 1)];
        }
    }
    
    // Same for the last column
    if ((gid_x + 1 < Width) && (lid_x == (TILE_X - 1)))
    {
        tile[lid_y + 1][TILE_X + 1] = d_Src[gid_y * Pitch + (gid_x + 1)];
    
        // Upper-right corner
        if ((gid_y > 0) && (lid_y == 0)) 
        {
            tile[0][TILE_X + 1] = d_Src[(gid_y - 1) * Pitch + (gid_x + 1)];
        }

        // Lower-right corner
        if ((gid_y < (Height - 1)) && (lid_y == (TILE_Y - 1)))
        {
            tile[TILE_Y + 1][TILE_X + 1] = d_Src[(gid_y + 1) * Pitch + (gid_x + 1)];
        }
    }

    // First row
    if ((gid_y > 0) && (lid_y == 0))
    {
        tile[0][lid_x + 1] = d_Src[(gid_y - 1) * Pitch + gid_x];
    }

    // Last row
    if ((gid_y < (Height - 1)) && (lid_y == (TILE_Y - 1)))
    {
        tile[TILE_Y + 1][lid_x + 1] = d_Src[(gid_y + 1) * Pitch + gid_x];
    }

    // Sync threads
    barrier(CLK_LOCAL_MEM_FENCE);

    size_t l_row = lid_y + 1;
    size_t l_col = lid_x + 1;

    float cell_magnitude = tile[l_row][l_col];

    switch (theta[gid_y * Pitch + gid_x])
    {
        // East and West
        case 0:
            // Suppress if the neighbour has a larger magnitude
            if (cell_magnitude <= tile[l_row][l_col+1] || 
                cell_magnitude <= tile[l_row][l_col-1])   
            {
                d_Dst[gid_y * Pitch + gid_x] = 0;
            }
            // Otherwise Write to output
            else
            {
                d_Dst[gid_y * Pitch + gid_x] = cell_magnitude;
            }
            break;
        // North East and South West
        case 45:
            if (cell_magnitude <= tile[l_row-1][l_col+1] || 
                cell_magnitude <= tile[l_row+1][l_col-1])   
            {
                d_Dst[gid_y * Pitch + gid_x] = 0;
            }
            else
            {
                d_Dst[gid_y * Pitch + gid_x] = cell_magnitude;
            }
            break;
                   
        // North and South
        case 90:
            if (cell_magnitude <= tile[l_row-1][l_col] || 
                cell_magnitude <= tile[l_row+1][l_col])   
            {
                d_Dst[gid_y * Pitch + gid_x] = 0;
            }
            else
            {
                d_Dst[gid_y * Pitch + gid_x] = cell_magnitude;
            }
            break;
        
        //North West and South East
        case 135:
            if (cell_magnitude <= tile[l_row-1][l_col-1] || 
                cell_magnitude <= tile[l_row+1][l_col+1])   
            {
                d_Dst[gid_y * Pitch + gid_x] = 0;
            }
            else
            {
                d_Dst[gid_y * Pitch + gid_x] = cell_magnitude;
            }
            break;
        default:
            d_Dst[gid_y * Pitch + gid_x] = cell_magnitude;
            break;
    }
}
