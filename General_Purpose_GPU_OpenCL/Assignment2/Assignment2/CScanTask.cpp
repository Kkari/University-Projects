/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CScanTask.h"

#include "../Common/CLUtil.h"
#include "../Common/CTimer.h"

#include <string.h>

using namespace std;

// number of banks in the local memory. This can be used to avoid bank conflicts
// but we also need to allocate more local memory for that.
#define NUM_BANKS	32

///////////////////////////////////////////////////////////////////////////////
// CScanTask

// only useful for debug info
const string g_kernelNames[2] = 
{
	"scanNaive",
	"scanWorkEfficient"
};

CScanTask::CScanTask(size_t ArraySize, size_t MinLocalWorkSize)
	: m_N(ArraySize), m_hArray(NULL), m_hResultCPU(NULL), m_hResultGPU(NULL),
	m_dPingArray(NULL), m_dPongArray(NULL),
	m_Program(NULL), 
	m_ScanNaiveKernel(NULL), m_ScanWorkEfficientKernel(NULL), m_ScanWorkEfficientAddKernel(NULL)
{
	// compute the number of levels that we need for the work-efficient algorithm

	m_MinLocalWorkSize = MinLocalWorkSize;

	m_nLevels = 1;
	size_t N = ArraySize;
	while (N > 0){
		N /= 2 * m_MinLocalWorkSize;
		m_nLevels++;
	}

	// Reset validation results
	for (int i = 0; i < (int)ARRAYLEN(m_bValidationResults); i++)
		m_bValidationResults[i] = false;
}

CScanTask::~CScanTask()
{
	ReleaseResources();
}

bool CScanTask::InitResources(cl_device_id Device, cl_context Context)
{
	//CPU resources
	m_hArray	 = new unsigned int[m_N];
	m_hResultCPU = new unsigned int[m_N];
	m_hResultGPU = new unsigned int[m_N];

	//fill the array with some values
	for(unsigned int i = 0; i < m_N; i++)
		m_hArray[i] = 1;			// Use this for debugging
		//m_hArray[i] = rand() & 15;

	//device resources
	// ping-pong buffers
	cl_int clError, clError2;
	m_dPingArray = clCreateBuffer(Context, CL_MEM_READ_WRITE, sizeof(cl_uint) * m_N, NULL, &clError2);
	clError = clError2;
	m_dPongArray = clCreateBuffer(Context, CL_MEM_READ_WRITE, sizeof(cl_uint) * m_N, NULL, &clError2);
	clError |= clError2;

	// level buffer
	m_dLevelArrays = new cl_mem[m_nLevels];
	unsigned int N = m_N;
	for (unsigned int i = 0; i < m_nLevels; i++) {
		m_dLevelArrays[i] = clCreateBuffer(Context, CL_MEM_READ_WRITE, sizeof(cl_uint) * N, NULL, &clError2);
		clError |= clError2;
		N = max(N / (2 * m_MinLocalWorkSize), m_MinLocalWorkSize);
	}
	V_RETURN_FALSE_CL(clError, "Error allocating device arrays");

	//load and compile kernels
	string programCode;

	CLUtil::LoadProgramSourceToMemory("Scan.cl", programCode);
	m_Program = CLUtil::BuildCLProgramFromMemory(Device, Context, programCode);
	if(m_Program == nullptr) return false;

	//create kernels
	m_ScanNaiveKernel = clCreateKernel(m_Program, "Scan_Naive", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create kernel.");

	m_ScanWorkEfficientKernel = clCreateKernel(m_Program, "Scan_WorkEfficient", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create kernel.");

	m_ScanWorkEfficientAddKernel = clCreateKernel(m_Program, "Scan_WorkEfficientAdd", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create kernel.");

	return true;
}

void CScanTask::ReleaseResources()
{
	// host resources
	SAFE_DELETE_ARRAY(m_hArray);

	SAFE_DELETE_ARRAY(m_hResultCPU);
	SAFE_DELETE_ARRAY(m_hResultGPU);

	// device resources
	SAFE_RELEASE_MEMOBJECT(m_dPingArray);
	SAFE_RELEASE_MEMOBJECT(m_dPongArray);

	if(m_dLevelArrays)
		for (unsigned int i = 0; i < m_nLevels; i++) {
			SAFE_RELEASE_MEMOBJECT(m_dLevelArrays[i]);
		}
	SAFE_DELETE_ARRAY(m_dLevelArrays);

	SAFE_RELEASE_KERNEL(m_ScanNaiveKernel);
	SAFE_RELEASE_KERNEL(m_ScanWorkEfficientKernel);
	SAFE_RELEASE_KERNEL(m_ScanWorkEfficientAddKernel);

	SAFE_RELEASE_PROGRAM(m_Program);
}

void CScanTask::ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{
	cout << endl;

	ValidateTask(Context, CommandQueue, LocalWorkSize, 0);
	ValidateTask(Context, CommandQueue, LocalWorkSize, 1);

	cout << endl;

	TestPerformance(Context, CommandQueue, LocalWorkSize, 0);
	TestPerformance(Context, CommandQueue, LocalWorkSize, 1);

	cout << endl;
}

void CScanTask::ComputeCPU()
{
	CTimer timer;
	timer.Start();
/*
    printf("\nArray: ");
    for (uint32_t i = 0; i < m_N; i++) 
    {
        printf("%d, ", m_hArray[i]); 
    }*/
	unsigned int nIterations = 1;
	for(unsigned int j = 0; j < nIterations; j++) {
		unsigned int sum = 0;
		for(unsigned int i = 0; i < m_N; i++) {
			sum += m_hArray[i];
			m_hResultCPU[i] = sum; 
		}
	}

   /* printf("\n\nCPU Result Array: ");
    for (uint32_t i = 0; i < m_N; i++) 
    {
        printf("%d, ", m_hResultCPU[i]); 
    }

    printf("\n\n");*/

	timer.Stop();
	double ms = timer.GetElapsedMilliseconds() / double(nIterations);
	cout << "  average time: " << ms << " ms, throughput: " << 1.0e-6 * (double)m_N / ms << " Gelem/s" <<endl;
}

bool CScanTask::ValidateResults()
{
	bool success = true;

	for(int i = 0; i < 2; i++)
		if(!m_bValidationResults[i])
		{
			cout<<"Validation of reduction kernel "<<g_kernelNames[i]<<" failed." << endl;
			success = false;
		}

	return success;
}

void CScanTask::Scan_Naive(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{

	// TO DO: Implement naive version of scan

	// NOTE: make sure that the final result is always in the variable m_dPingArray
	// as this is read back for the correctness check
	// (CReductionTask::ValidateTask)
	//
	// hint: for example, you can use swap(m_dPingArray, m_dPongArray) at the end of your for loop...
    
    size_t localWorkSize = LocalWorkSize[0];
    uint32_t offset = 1;
    size_t globalWorkSize = CLUtil::GetGlobalWorkSize(m_N, localWorkSize);

    int clErr = 0;
    uint32_t limit = log2(globalWorkSize);
    bool needs_swap = false;
    uint32_t temp_gws = globalWorkSize;

    for (uint32_t i = 0; i < limit; i++) 
    {

        
        clErr |= clSetKernelArg(m_ScanNaiveKernel, 0, sizeof(cl_mem), (void*) &m_dPingArray);
        clErr |= clSetKernelArg(m_ScanNaiveKernel, 1, sizeof(cl_mem), (void*) &m_dPongArray);
        V_RETURN_CL(clErr, "Error binding array variables!");

        globalWorkSize = temp_gws - offset;
        uint32_t passed_gws = globalWorkSize;
        globalWorkSize = CLUtil::GetGlobalWorkSize(globalWorkSize, localWorkSize);

        if (globalWorkSize < localWorkSize) 
        {
            localWorkSize = globalWorkSize;
        }


        clErr |= clSetKernelArg(m_ScanNaiveKernel, 2, sizeof(cl_uint), (void*)&passed_gws);
        clErr |= clSetKernelArg(m_ScanNaiveKernel, 3, sizeof(cl_uint), (void*)&offset);
        V_RETURN_CL(clErr, "Error binding offset variable!");
       
        clErr = clEnqueueNDRangeKernel(
                    CommandQueue, 
                    m_ScanNaiveKernel,
                    1, NULL, &globalWorkSize, &localWorkSize, 0,
                    NULL, NULL);

        V_RETURN_CL(clErr, "Error executing kernel!");
    
        offset = (offset << 1);
        std::swap(m_dPingArray, m_dPongArray);
        needs_swap = !needs_swap;
    }

    if (needs_swap) 
    {
        std::swap(m_dPingArray, m_dPongArray);
    }

 /*   clErr = clEnqueueReadBuffer(
            CommandQueue, m_dPingArray, CL_TRUE, 0, 
            sizeof(cl_uint) * m_N, m_hResultGPU, 0, NULL, NULL);

    V_RETURN_CL(clErr, "Read back failed!");
*/
    /*printf("Result GPU: ");
    for(uint32_t i = 0; i < m_N; i++)
    {
      printf("%u, ", m_hResultGPU[i]);
    }*/
}

void CScanTask::Scan_WorkEfficient(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{

	// TO DO: Implement efficient version of scan

	// Make sure that the local prefix sum works before you start experimenting with large arrays

	cl_int clErr = 0;
	size_t localWorkSize[1] { LocalWorkSize[0] };
	size_t globalWorkSize[1] { CLUtil::GetGlobalWorkSize(m_N, localWorkSize[0]) };

    // it's half of the original size, so we start half as many threads
	unsigned int elemCount = m_N >> 1;

	clErr = clEnqueueWriteBuffer(CommandQueue, m_dLevelArrays[0], CL_FALSE, 0, m_N * sizeof(cl_uint), m_hArray, 0, NULL, NULL);
	V_RETURN_CL(clErr, "Failed to write array to device: WorkEff");

	// running the ordinary kernels, building the tree
    for (unsigned int i = 0; i < m_nLevels - 1; elemCount /= localWorkSize[0], ++i) 
	{
		// we control the global worksize with elemCount
        globalWorkSize[0] = CLUtil::GetGlobalWorkSize(elemCount, localWorkSize[0]);

		clErr = clSetKernelArg(m_ScanWorkEfficientKernel, 0, sizeof(cl_mem), (void*)&m_dLevelArrays[i]);
		clErr |= clSetKernelArg(m_ScanWorkEfficientKernel, 1, sizeof(cl_mem), (void*)&m_dLevelArrays[i + 1]);
		clErr |= clSetKernelArg(m_ScanWorkEfficientKernel, 2, (localWorkSize[0] << 1) * sizeof(cl_uint)  + 32 * sizeof(cl_uint), NULL);
		V_RETURN_CL(clErr, "Failed to set kernel args: WorkEff.");

		clErr = clEnqueueNDRangeKernel(CommandQueue, 
                                        m_ScanWorkEfficientKernel, 1, NULL, 
                                        globalWorkSize, localWorkSize, 0, NULL, NULL);
		V_RETURN_CL(clErr, "Failed to execute kernel: WorkEff.");
	}
	
    // elem size is at one, let's heal it up a bit (which will be the element count of the last higher array)
    elemCount *= localWorkSize[0];

    // merging the levels in the tree
	for (unsigned int i = m_nLevels - 1; i > 0; elemCount *= localWorkSize[0], --i) 
	{
		// same as up there, but in the other direction
        globalWorkSize[0] = CLUtil::GetGlobalWorkSize(elemCount, localWorkSize[0]);
		
        clErr = clSetKernelArg(m_ScanWorkEfficientAddKernel, 0, 
                                sizeof(cl_mem), (void*)&m_dLevelArrays[i]);
		
        clErr |= clSetKernelArg(m_ScanWorkEfficientAddKernel, 1, 
                                sizeof(cl_mem), (void*)&m_dLevelArrays[i - 1]);
		
        clErr |= clSetKernelArg(m_ScanWorkEfficientAddKernel, 2,
                                sizeof(cl_uint), NULL);
		
        V_RETURN_CL(clErr, "Failed to set kernel args: WorkEffAdd.");

		clErr = clEnqueueNDRangeKernel(CommandQueue, 
                                        m_ScanWorkEfficientAddKernel, 1, NULL, 
                                        globalWorkSize, localWorkSize, 0, NULL, NULL);
		V_RETURN_CL(clErr, "Failed to execute kernel: WorkEffAdd.");
	}

	clErr = clEnqueueReadBuffer(CommandQueue, m_dLevelArrays[0], CL_TRUE, 0, m_N * sizeof(cl_uint), m_hResultGPU, 0, NULL, NULL);
	V_RETURN_CL(clErr, "Failed to read result from device: WorkEffAdd.");
}


void CScanTask::ValidateTask(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3], unsigned int Task)
{
	//run selected task
	switch (Task){
		case 0:
			V_RETURN_CL(clEnqueueWriteBuffer(CommandQueue, m_dPingArray, CL_FALSE, 0, m_N * sizeof(cl_uint), m_hArray, 0, NULL, NULL), "Error copying data from host to device!");
			Scan_Naive(Context, CommandQueue, LocalWorkSize);
			V_RETURN_CL(clEnqueueReadBuffer(CommandQueue, m_dPingArray, CL_TRUE, 0, m_N * sizeof(cl_uint), m_hResultGPU, 0, NULL, NULL), "Error reading data from device!");
			break;
		case 1:
			V_RETURN_CL(clEnqueueWriteBuffer(CommandQueue, m_dLevelArrays[0], CL_FALSE, 0, m_N * sizeof(cl_uint), m_hArray, 0, NULL, NULL), "Error copying data from host to device!");
			Scan_WorkEfficient(Context, CommandQueue, LocalWorkSize);
			V_RETURN_CL(clEnqueueReadBuffer(CommandQueue, m_dLevelArrays[0], CL_TRUE, 0, m_N * sizeof(cl_uint), m_hResultGPU, 0, NULL, NULL), "Error reading data from device!");
			break;
	}

	// validate results
	m_bValidationResults[Task] =( memcmp(m_hResultCPU, m_hResultGPU, m_N * sizeof(unsigned int)) == 0);
}

void CScanTask::TestPerformance(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3], unsigned int Task)
{
	cout << "Testing performance of task " << g_kernelNames[Task] << endl;

	//write input data to the GPU
	V_RETURN_CL(clEnqueueWriteBuffer(CommandQueue, m_dPingArray, CL_FALSE, 0, m_N * sizeof(cl_uint), m_hArray, 0, NULL, NULL), "Error copying data from host to device!");
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
				Scan_Naive(Context, CommandQueue, LocalWorkSize);
				break;
			case 1:
				Scan_WorkEfficient(Context, CommandQueue, LocalWorkSize);
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
