#define lid_x get_local_id(0)
#define lid_y get_local_id(1)
#define gid_x get_global_id(0)
#define gid_y get_global_id(1)
#define lSize_x get_local_size(0)
#define lSize_y get_local_size(1)

#define KERNEL_WIDTH 3

__constant int sobelx[3][3] = { { -1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
__constant int sobely[3][3] = { { -1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

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

// With & Height are the image dimensions (should be multiple of the tile size)
__kernel __attribute__((reqd_work_group_size(TILE_X, TILE_Y, 1)))
void Sobel(
				__global float* d_Dst,
				__global uchar* theta,
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

    //tile[lid_y + 1][lid_x + 1] = d_Src[gid_y * Pitch + gid_x];
	
	if ((gid_x < Width) && (gid_y < Height)) {

        if (gid_y < Height)
        {
		    tile[lid_y + 1][lid_x + 1] = d_Src[gid_y * Pitch + gid_x];
        } 
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

    // Filling up the halo with the same elements, so they won't be preceived as edges.
    if (gid_y == 0) 
    {
        tile[0][lid_x] = tile[1][lid_x];
        
        if (lid_x == 0 && lid_y <= 1)
        {
            tile[0][TILE_X + lid_y] = tile[1][TILE_X + lid_y];
        }
    }

    if (gid_y == Height)
    {
        tile[TILE_Y + 1][lid_x] = tile[TILE_Y][lid_x];
        
        if (lid_x == 0 && lid_y <= 1)
        {
            tile[TILE_Y + 1][TILE_X + lid_y] = tile[TILE_Y + 1][TILE_X + lid_y];
        }
    }

    if (gid_x == 0)
    {
        tile[lid_y][0] = tile[lid_y][1];

        if (lid_x == 0 && lid_y <= 1)
        {
            tile[TILE_Y + lid_y][0] = tile[TILE_Y + lid_y][1];
        }
    }

    if (gid_x == Width)
    {
        tile[lid_y][TILE_X + 1] = tile[lid_y][TILE_X];

        if (lid_x == 0 && lid_y <= 1)
        {
            tile[TILE_Y + lid_y][TILE_X + 1] = tile[TILE_Y + lid_y][TILE_X];
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
	// Perform the convolution and store the convolved signal to d_Dst.
    
    float sumx = 0, sumy = 0, angle = 0;

    for (uint i = 0; i < KERNEL_WIDTH; i++)
    {
        #pragma unroll
        for (uint j = 0; j < KERNEL_WIDTH; j++)
        {
          sumx += tile[lid_y + i][lid_x + j] * sobelx[i][j];
          sumy += tile[lid_y + i][lid_x + j] * sobely[i][j];
        }
    }

    d_Dst[gid_y * Pitch + gid_x] = min(255.f, max(0.f, hypot(sumx, sumy)));

    angle = atan2(sumy, sumx);

    if (angle < 0)
    {
        angle = fmod((angle + 2 * M_PI), (2 * M_PI));
    }

    theta[gid_y * Pitch + gid_x] = ((int)(degrees(angle * (M_PI/8) + M_PI / 8 - 0.0001) / 45) * 45) % 100;
}
