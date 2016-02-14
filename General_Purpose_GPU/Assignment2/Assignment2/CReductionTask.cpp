/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CReductionTask.h"

#include "../Common/CLUtil.h"
#include "../Common/CTimer.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CReductionTask

string g_kernelNames[4] = {
	"interleavedAddressing",
	"sequentialAddressing",
	"kernelDecomposition",
	"kernelDecompositionUnroll"
};

CReductionTask::CReductionTask(size_t ArraySize)
	: m_N(ArraySize), m_hInput(NULL), 
	m_dPingArray(NULL),
	m_dPongArray(NULL),
	m_Program(NULL), 
	m_InterleavedAddressingKernel(NULL), m_SequentialAddressingKernel(NULL), m_DecompKernel(NULL), m_DecompUnrollKernel(NULL)
{
}

CReductionTask::~CReductionTask()
{
	ReleaseResources();
}

bool CReductionTask::InitResources(cl_device_id Device, cl_context Context)
{
	//CPU resources
	m_hInput = new unsigned int[m_N];

	//fill the array with some values
	for(unsigned int i = 0; i < m_N; i++) 
		//m_hInput[i] = 1;			// Use this for debugging
		m_hInput[i] = rand() & 15;

	//device resources
	cl_int clError, clError2;
	m_dPingArray = clCreateBuffer(Context, CL_MEM_READ_WRITE, sizeof(cl_uint) * m_N, NULL, &clError2);
	clError = clError2;
	m_dPongArray = clCreateBuffer(Context, CL_MEM_READ_WRITE, sizeof(cl_uint) * m_N, NULL, &clError2);
	clError |= clError2;
	V_RETURN_FALSE_CL(clError, "Error allocating device arrays");

	//load and compile kernels
	string programCode;

	CLUtil::LoadProgramSourceToMemory("Reduction.cl", programCode);
	m_Program = CLUtil::BuildCLProgramFromMemory(Device, Context, programCode);
	if(m_Program == nullptr) return false;

	//create kernels
	m_InterleavedAddressingKernel = clCreateKernel(m_Program, "Reduction_InterleavedAddressing", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create kernel: Reduction_InterleavedAddressing.");

	m_SequentialAddressingKernel = clCreateKernel(m_Program, "Reduction_SequentialAddressing", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create kernel: Reduction_SequentialAddressing.");

	m_DecompKernel = clCreateKernel(m_Program, "Reduction_Decomp", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create kernel: Reduction_Decomp.");

	m_DecompUnrollKernel = clCreateKernel(m_Program, "Reduction_DecompUnroll", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create kernel: Reduction_DecompUnroll.");

	return true;
}

void CReductionTask::ReleaseResources()
{
	// host resources
	SAFE_DELETE_ARRAY(m_hInput);

	// device resources
	SAFE_RELEASE_MEMOBJECT(m_dPingArray);
	SAFE_RELEASE_MEMOBJECT(m_dPongArray);

	SAFE_RELEASE_KERNEL(m_InterleavedAddressingKernel);
	SAFE_RELEASE_KERNEL(m_SequentialAddressingKernel);
	SAFE_RELEASE_KERNEL(m_DecompKernel);
	SAFE_RELEASE_KERNEL(m_DecompUnrollKernel);

	SAFE_RELEASE_PROGRAM(m_Program);
}

void CReductionTask::ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{
	ExecuteTask(Context, CommandQueue, LocalWorkSize, 0);
	ExecuteTask(Context, CommandQueue, LocalWorkSize, 1);
	ExecuteTask(Context, CommandQueue, LocalWorkSize, 2);
	ExecuteTask(Context, CommandQueue, LocalWorkSize, 3);

	TestPerformance(Context, CommandQueue, LocalWorkSize, 0);
	TestPerformance(Context, CommandQueue, LocalWorkSize, 1);
	TestPerformance(Context, CommandQueue, LocalWorkSize, 2);
	TestPerformance(Context, CommandQueue, LocalWorkSize, 3);

}

void CReductionTask::ComputeCPU()
{
	CTimer timer;
	timer.Start();

	printf("\n");
    for(unsigned int i = 0; i < m_N; i++) {
 //       printf("%d, ", m_hInput[i]); 
    }

    unsigned int nIterations = 10;
	for(unsigned int j = 0; j < nIterations; j++) {
		m_resultCPU = m_hInput[0];
		for(unsigned int i = 1; i < m_N; i++) {
			m_resultCPU += m_hInput[i]; 
		}
	}

	timer.Stop();

    cout << "\n\nCPU result: " << m_resultCPU << endl << endl;

	double ms = timer.GetElapsedMilliseconds() / double(nIterations);
	cout << "  average time: " << ms << " ms, throughput: " << 1.0e-6 * (double)m_N / ms << " Gelem/s" <<endl;
}

bool CReductionTask::ValidateResults()
{
	bool success = true;

	for(int i = 0; i < 4; i++)
		if(m_resultGPU[i] != m_resultCPU)
		{
			cout<<"Validation of reduction kernel "<<g_kernelNames[i]<<" failed." << endl;
			success = false;
		}

	return success;
}

void CReductionTask::Reduction_InterleavedAddressing(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{
	cl_int clErr;
	//size_t globalWorkSize[1];
	size_t localWorkSize = LocalWorkSize[0];
	//unsigned int stride = ...;
    
    size_t globalWorkSize = CLUtil::GetGlobalWorkSize(m_N, LocalWorkSize[0]);
    
 //   cout << "Executing " << globalWorkSize << " threads in " << nGroups << " groups of size " << LocalWorkSize[0] << endl;

	clErr = clSetKernelArg(m_InterleavedAddressingKernel, 0, sizeof(cl_mem), (void*)&m_dPingArray);
	// TO DO: Implement reduction with interleaved addressing

    globalWorkSize = m_N / 2;

    for (uint32_t i = 0; i < (uint32_t) log2(m_N); i++){
    
        
        if (globalWorkSize < localWorkSize) 
        {
            localWorkSize = globalWorkSize;
        }

        globalWorkSize = CLUtil::GetGlobalWorkSize(globalWorkSize, localWorkSize);
        
        clErr = clSetKernelArg(m_InterleavedAddressingKernel, 1, sizeof(cl_uint), (void*)&i);
        
        V_RETURN_CL(clErr, "Error binding stride variable");

        clErr = clEnqueueNDRangeKernel(
                    CommandQueue, 
                    m_InterleavedAddressingKernel,
                    1, NULL, &globalWorkSize, &localWorkSize, 0,
                    NULL, NULL);

        V_RETURN_CL(clErr, "Error executing kernel!");
        
        globalWorkSize /= 2;
    }
    
    clErr = clEnqueueReadBuffer(
            CommandQueue, m_dPingArray, CL_TRUE, 0, 
            sizeof(int), m_resultGPU, 0, NULL, NULL);

    printf("\n\n GPU RESULT: %d\n\n", m_resultGPU[0]);
}

void CReductionTask::Reduction_SequentialAddressing(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{
	cl_int clErr;
	//size_t globalWorkSize[1];
	size_t localWorkSize = LocalWorkSize[0];
	//unsigned int stride = ...;
    
    size_t globalWorkSize = CLUtil::GetGlobalWorkSize(m_N, localWorkSize);
    
 //   cout << "Executing " << globalWorkSize << " threads in " << nGroups << " groups of size " << LocalWorkSize[0] << endl;

	clErr = clSetKernelArg(m_SequentialAddressingKernel, 0, sizeof(cl_mem), (void*)&m_dPingArray);
	
    V_RETURN_CL(clErr, "Error binding array variable");
    // TO DO: Implement reduction with interleaved addressing

    globalWorkSize = m_N / 2;

    for (uint32_t i = 0; i < (uint32_t) log2(m_N); i++){
        
        
        if (globalWorkSize < localWorkSize) 
        {
            localWorkSize = globalWorkSize;
        }

        globalWorkSize = CLUtil::GetGlobalWorkSize(globalWorkSize, localWorkSize);

        clErr = clSetKernelArg(m_SequentialAddressingKernel, 1, sizeof(cl_uint), (void*)&globalWorkSize);
        
        V_RETURN_CL(clErr, "Error binding stride variable");

        clErr = clEnqueueNDRangeKernel(
                    CommandQueue, 
                    m_SequentialAddressingKernel,
                    1, NULL, &globalWorkSize, &localWorkSize, 0,
                    NULL, NULL);

        V_RETURN_CL(clErr, "Error executing kernel!");
        
        globalWorkSize /= 2;
    }
    
    clErr = clEnqueueReadBuffer(
            CommandQueue, m_dPingArray, CL_TRUE, 0, 
            sizeof(int), &(m_resultGPU[1]), 0, NULL, NULL);

    printf("\n\n GPU RESULT: %d\n\n", m_resultGPU[1]);
}

void CReductionTask::Reduction_Decomp(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{

	// TO DO: Implement reduction with kernel decomposition

	// NOTE: make sure that the final result is always in the variable m_dPingArray
	// as this is read back for the correctness check
	// (CReductionTask::ExecuteTask)
	//
	// hint: for example, you can use swap(m_dPingArray, m_dPongArray) at the end of your for loop...
	
    cl_int clErr;
	size_t localWorkSize = LocalWorkSize[0]; 
    size_t globalWorkSize = CLUtil::GetGlobalWorkSize(m_N, LocalWorkSize[0]);
    
    // cout << "Executing " << globalWorkSize << " threads in " << nGroups << " groups of size " << LocalWorkSize[0] << endl;

    clErr = clSetKernelArg(m_DecompKernel, 3, localWorkSize * sizeof(float), NULL);
    
    V_RETURN_CL(clErr, "Error binding array variables");

    // The number of iterations is nothing else as the number of blocks we need to process half of the array.
    // It's half of it, because before the looping in the kernel we add the first pairs together as in sequential addressing.
    // This way we need to run less kernels.
    uint32_t num_iterations = ceil(log2(m_N) / (double)log2(2 * localWorkSize));
    
    // The gws is half of the array length at first.
    globalWorkSize = m_N / 2;

    // If we have an odd number of kernels, then we should also swap once in the end, 
    // so we can read the result from the Ping Array.
    bool needs_swap = (globalWorkSize % 2 == 1);

    // printf("\nnum_iterations: %d\n\n", num_iterations);

    for (uint32_t i = 0; i < num_iterations; i++){
    
    // As we swap the variables we have to bind the again to the kernel.
	clErr = clSetKernelArg(m_DecompKernel, 0, sizeof(cl_mem), (void*) &m_dPingArray);
    clErr = clSetKernelArg(m_DecompKernel, 1, sizeof(cl_mem), (void*) &m_dPongArray);
        
        // If the GWS is less than the LWS, then we adjust the lws to have the right amount of threads to run.
        // It is the case at the last kernel, this way we only launch exactly one kernel with the right amount of
        // threads.
        if (globalWorkSize < localWorkSize) 
        {
            localWorkSize = globalWorkSize;
        }

        // Check, although because of the restriction, that the arrays have the length 2^x it could be ommitted.
        globalWorkSize = CLUtil::GetGlobalWorkSize(globalWorkSize, localWorkSize);
        
        // Here we get the number of inner loops that are needed in each kernel to produce the output. 
        uint32_t number_of_inner_loops = log2(localWorkSize);

        // printf("\nit_inner: %u lws: %lu gws: %lu\n", number_of_inner_loops, localWorkSize, globalWorkSize);
        clErr = clSetKernelArg(m_DecompKernel, 2, sizeof(cl_uint), (void*)&number_of_inner_loops); 
        
        V_RETURN_CL(clErr, "Error binding stride variable");

        clErr = clEnqueueNDRangeKernel(
                    CommandQueue, 
                    m_DecompKernel,
                    1, NULL, &globalWorkSize, &localWorkSize, 0,
                    NULL, NULL);

        V_RETURN_CL(clErr, "Error executing kernel!");
        
        // We divide the gws. Because the output array will have the length of lws, but we need only half the threads
        // to process it again, because of the initial addition in the kernel.
        globalWorkSize = (globalWorkSize / (2 * localWorkSize));

        // Swap the guys, so we won't have any problems with the race conditions. These cl_mem stuff are like file descriptors
        // in Linux, so they aren't pointers, but rather integers that are used internally to identify resources.
        std::swap(m_dPingArray, m_dPongArray);
    }
    
    // Swap if the solution is in the wrong array.
    if (needs_swap) 
    {
        std::swap(m_dPingArray, m_dPongArray);
    }

    clErr = clEnqueueReadBuffer(
            CommandQueue, m_dPingArray, CL_TRUE, 0, 
            sizeof(int), &(m_resultGPU[2]), 0, NULL, NULL);

    printf("\n\n GPU RESULT: %d\n\n", m_resultGPU[2]);
}


void CReductionTask::Reduction_DecompUnroll(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{

	// TO DO: Implement reduction with loop unrolling

	// NOTE: make sure that the final result is always in the variable m_dPingArray
	// as this is read back for the correctness check
	// (CReductionTask::ExecuteTask)
	//
	// hint: for example, you can use swap(m_dPingArray, m_dPongArray) at the end of your for loop...
	
    cl_int clErr;
	size_t localWorkSize = LocalWorkSize[0]; 
    size_t globalWorkSize = CLUtil::GetGlobalWorkSize(m_N, LocalWorkSize[0]);
    
    // cout << "Executing " << globalWorkSize << " threads in " << nGroups << " groups of size " << LocalWorkSize[0] << endl;

	clErr = clSetKernelArg(m_DecompUnrollKernel, 0, sizeof(cl_mem), (void*) &m_dPingArray);
    clErr = clSetKernelArg(m_DecompUnrollKernel, 1, sizeof(cl_mem), (void*) &m_dPongArray);
    clErr = clSetKernelArg(m_DecompUnrollKernel, 3, localWorkSize * sizeof(float), NULL);
    
    V_RETURN_CL(clErr, "Error binding array variables");

    // The number of iterations is nothing else as the number of blocks we need to process half of the array.
    // It's half of it, because before the looping in the kernel we add the first pairs together as in sequential addressing.
    // This way we need to run less kernels.
    uint32_t num_iterations = ceil(log2(m_N) / (double)log2(2 * localWorkSize));
    
    // The gws is half of the array length at first.
    globalWorkSize = m_N / 2;

    // If we have an odd number of kernels, then we should also swap once in the end, 
    // so we can read the result from the Ping Array.
    bool needs_swap = (globalWorkSize % 2 == 1);

    // printf("\nnum_iterations: %d\n\n", num_iterations);

    for (uint32_t i = 0; i < num_iterations; i++){
    
    // As we swap the variables we have to bind the again to the kernel.
	clErr = clSetKernelArg(m_DecompUnrollKernel, 0, sizeof(cl_mem), (void*) &m_dPingArray);
    clErr = clSetKernelArg(m_DecompUnrollKernel, 1, sizeof(cl_mem), (void*) &m_dPongArray);
        
        // If the GWS is less than the LWS, then we adjust the lws to have the right amount of threads to run.
        // It is the case at the last kernel, this way we only launch exactly one kernel with the right amount of
        // threads.
        if (globalWorkSize < localWorkSize) 
        {
            localWorkSize = globalWorkSize;
        }

        // Check, although because of the restriction, that the arrays have the length 2^x it could be ommitted.
        globalWorkSize = CLUtil::GetGlobalWorkSize(globalWorkSize, localWorkSize);
        
        // Here we get the number of inner loops that are needed in each kernel to produce the output. 
        uint32_t number_of_inner_loops = log2(localWorkSize);

        // printf("\nit_inner: %u lws: %lu gws: %lu\n", number_of_inner_loops, localWorkSize, globalWorkSize);
        clErr = clSetKernelArg(m_DecompUnrollKernel, 2, sizeof(cl_uint), (void*)&number_of_inner_loops); 
        
        V_RETURN_CL(clErr, "Error binding stride variable");

        clErr = clEnqueueNDRangeKernel(
                    CommandQueue, 
                    m_DecompUnrollKernel,
                    1, NULL, &globalWorkSize, &localWorkSize, 0,
                    NULL, NULL);

        V_RETURN_CL(clErr, "Error executing kernel!");
        
        // We divide the gws. Because the output array will have the length of lws, but we need only half the threads
        // to process it again, because of the initial addition in the kernel.
        globalWorkSize = (globalWorkSize / (2 * localWorkSize));

        // Swap the guys, so we won't have any problems with the race conditions. These cl_mem stuff are like file descriptors
        // in Linux, so they aren't pointers, but rather integers that are used internally to identify resources.
        std::swap(m_dPingArray, m_dPongArray);
    }
    
    // Swap if the solution is in the wrong array.
    if (needs_swap) 
    {
        std::swap(m_dPingArray, m_dPongArray);
    }

    clErr = clEnqueueReadBuffer(
            CommandQueue, m_dPingArray, CL_TRUE, 0, 
            sizeof(int), &(m_resultGPU[3]), 0, NULL, NULL);

    printf("\n\n GPU RESULT: %d\n\n", m_resultGPU[3]);
}

void CReductionTask::ExecuteTask(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3], unsigned int Task)
{
	//write input data to the GPU
	V_RETURN_CL(clEnqueueWriteBuffer(CommandQueue, m_dPingArray, CL_FALSE, 0, m_N * sizeof(cl_uint), m_hInput, 0, NULL, NULL), "Error copying data from host to device!");

	//run selected task
	switch (Task){
		case 0:
			Reduction_InterleavedAddressing(Context, CommandQueue, LocalWorkSize);
			break;
		case 1:
			Reduction_SequentialAddressing(Context, CommandQueue, LocalWorkSize);
			break;
		case 2:
			Reduction_Decomp(Context, CommandQueue, LocalWorkSize);
			break;
		case 3:
			Reduction_DecompUnroll(Context, CommandQueue, LocalWorkSize);
			break;

	}

	//read back the results synchronously.
	m_resultGPU[Task] = 0;
	V_RETURN_CL(clEnqueueReadBuffer(CommandQueue, m_dPingArray, CL_TRUE, 0, 1 * sizeof(cl_uint), &m_resultGPU[Task], 0, NULL, NULL), "Error reading data from device!");
	
}

void CReductionTask::TestPerformance(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3], unsigned int Task)
{
	cout << "Testing performance of task " << g_kernelNames[Task] << endl;

	//write input data to the GPU
	V_RETURN_CL(clEnqueueWriteBuffer(CommandQueue, m_dPingArray, CL_FALSE, 0, m_N * sizeof(cl_uint), m_hInput, 0, NULL, NULL), "Error copying data from host to device!");
	//finish all before we start meassuring the time
	V_RETURN_CL(clFinish(CommandQueue), "Error finishing the queue!");

	CTimer timer;
	timer.Start();

	//run the kernel N times
	unsigned int nIterations = 1;
	for(unsigned int i = 0; i < nIterations; i++) {
		//run selected task
		switch (Task){
			case 0:
				Reduction_InterleavedAddressing(Context, CommandQueue, LocalWorkSize);
				break;
			case 1:
				Reduction_SequentialAddressing(Context, CommandQueue, LocalWorkSize);
				break;
			case 2:
				Reduction_Decomp(Context, CommandQueue, LocalWorkSize);
				break;
			case 3:
				Reduction_DecompUnroll(Context, CommandQueue, LocalWorkSize);
				break;
		}
	}

	//wait until the command queue is empty again
	V_RETURN_CL(clFinish(CommandQueue), "Error finishing the queue!");

	timer.Stop();

	double ms = timer.GetElapsedMilliseconds() / double(nIterations);
	cout << "  average time: " << ms << " ms, throughput: " << 1.0e-6 * (double)m_N / ms << " Gelem/s" <<endl;
}

///////////////////////////////////////////////////////////////////////////////
