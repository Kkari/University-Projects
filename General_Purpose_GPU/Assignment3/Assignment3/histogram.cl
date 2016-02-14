#define lid_x get_local_id(0)
#define lid_y get_local_id(1)

#define gid_x get_global_id(0)
#define gid_y get_global_id(1)

#define lSize_x get_local_size(0)
#define lSize_y get_local_size(1)

__kernel void
set_array_to_constant(
	__global int *array,
	int num_elements,
	int val
)
{
	// There is no need to touch this kernel
	if(get_global_id(0) < num_elements)
		array[get_global_id(0)] = val;
}

__kernel void
compute_histogram(
        __global int *histogram,   // accumulate histogram here
        __global const float *img, // input image
        int width,                 // image width
        int height,                // image height
        int pitch,                 // image pitch
        int num_hist_bins          // number of histogram bins
)
{
 
        // Insert your kernel code here
        int idx = 0;

        if (gid_x < width && gid_y < height)
        {
                float p = img[gid_y * pitch + gid_x] * (float)num_hist_bins;
                int res = max(0, (int)p);
                idx = min((num_hist_bins - 1), res);
 
                atomic_inc(&(histogram[idx]));
        }
}

__kernel void
compute_histogram_local_memory(
	__global int *histogram,   // accumulate histogram here
	__global const float *img, // input image
	int width,                 // image width
	int height,                // image height
	int pitch,                 // image pitch
	int num_hist_bins,         // number of histogram bins
	__local int *local_hist
)
{
        // Fun fact: it's slower on nvidia, but faster on an integrated intel,
        // my video card does too good job at hiding global access. :(
        int thread_index = lid_x + (lid_y * lSize_x);

        // Insert your kernel code here
        if (thread_index < num_hist_bins)
        {
            local_hist[thread_index] = 0;
        }

        barrier(CLK_LOCAL_MEM_FENCE);
        
        int idx = 0;
        if ((gid_x < width) && (gid_y < height))
        {
                float p = img[gid_y * pitch + gid_x] * (float) num_hist_bins;
                int res = max(0, (int) p);
                idx = min((num_hist_bins - 1), res);
 
                atomic_inc(&local_hist[idx]);
        }

        barrier(CLK_LOCAL_MEM_FENCE);

        if (thread_index < num_hist_bins)
        {
            atomic_add(&(histogram[thread_index]), local_hist[thread_index]);
        }
} 
