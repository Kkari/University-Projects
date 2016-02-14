/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CConvolutionSeparableTask.h"

#include "../Common/CLUtil.h"
#include "../Common/CTimer.h"

#include <sstream>
#include <cstring>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CConvolutionSeparableTask

CConvolutionSeparableTask::CConvolutionSeparableTask(
		const std::string& OutFileName, 
		const std::string& FileName, 
		size_t LocalSizeHorizontal[2],
		size_t LocalSizeVertical[2],
		int StepsHorizontal,
		int StepsVertical,
		int KernelRadius,
		float* pKernelHorizontal,
		float* pKernelVertical
)
	: CConvolutionTaskBase(FileName, true)
	, m_OutFileName(OutFileName)
	, m_StepsHorizontal(StepsHorizontal)
	, m_StepsVertical(StepsVertical)
	, m_KernelRadius(KernelRadius)
{
	m_LocalSizeHorizontal[0] = LocalSizeHorizontal[0];
	m_LocalSizeHorizontal[1] = LocalSizeHorizontal[1];
	m_LocalSizeVertical[0]   = LocalSizeVertical[0];
	m_LocalSizeVertical[1]   = LocalSizeVertical[1];

	const unsigned int kernelSize = 2 * m_KernelRadius + 1;
	m_hKernelHorizontal = new float[kernelSize];
	m_hKernelVertical = new float[kernelSize];
	memcpy(m_hKernelHorizontal, pKernelHorizontal, kernelSize * sizeof(float));
	memcpy(m_hKernelVertical, pKernelVertical, kernelSize * sizeof(float));

	m_dGPUWorkingBuffer = nullptr;
	m_hCPUWorkingBuffer = nullptr;

	m_FileNamePostfix = "Separable_" + OutFileName;
	m_ProgramName = "ConvolutionSeparable.cl";
}

CConvolutionSeparableTask::~CConvolutionSeparableTask()
{
	delete [] m_hKernelHorizontal;
	delete [] m_hKernelVertical;

	ReleaseResources();
}

bool CConvolutionSeparableTask::InitResources(cl_device_id Device, cl_context Context)
{
	if(!CConvolutionTaskBase::InitResources(Device, Context))
		return false;
	

	//create GPU resources
	//we can init the kernel buffer during creation as its contents will not change
	const unsigned int kernelSize = 2 * m_KernelRadius + 1;

	cl_int clError = 0;
	cl_int clErr;
	m_dKernelHorizontal = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, kernelSize * sizeof(cl_float), 
		m_hKernelHorizontal, &clErr);
	clError |= clErr;
	m_dKernelVertical = clCreateBuffer(Context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, kernelSize * sizeof(cl_float), 
		m_hKernelVertical, &clErr);
	clError |= clErr;
	V_RETURN_FALSE_CL(clError, "Error allocating device kernel constants.");

	m_dGPUWorkingBuffer = clCreateBuffer(Context, CL_MEM_READ_WRITE, m_Pitch * m_Height * sizeof(cl_float), NULL, &clError);
	V_RETURN_FALSE_CL(clError, "Error allocating device working array");

	m_dSobelAngleBuffer = clCreateBuffer(Context, CL_MEM_READ_WRITE, m_Pitch * m_Height * sizeof(cl_uchar), NULL, &clError);
	V_RETURN_FALSE_CL(clError, "Error allocating device working array");
	
    m_hCPUWorkingBuffer = new float[m_Height * m_Pitch];

    // gauss
	string programCode;
    string sobelCode;
    string nonMaxCode;
    string hystCode;

	CLUtil::LoadProgramSourceToMemory("ConvolutionSeparable.cl", programCode);
	CLUtil::LoadProgramSourceToMemory("Sobel.cl", sobelCode);
	CLUtil::LoadProgramSourceToMemory("NonMaxSuppression.cl", nonMaxCode);
	CLUtil::LoadProgramSourceToMemory("Hyst.cl", hystCode);

	//This time we define several kernel-specific constants that we did not know during
	//implementing the kernel, but we need to include during compile time.
	stringstream compileOptions;
	compileOptions<<"-cl-fast-relaxed-math"
	<<" -D KERNEL_RADIUS="<<m_KernelRadius
	<<" -D H_GROUPSIZE_X="<<m_LocalSizeHorizontal[0]<<" -D H_GROUPSIZE_Y="<<m_LocalSizeHorizontal[1]
	<<" -D H_RESULT_STEPS="<<m_StepsHorizontal
	<<" -D V_GROUPSIZE_X="<<m_LocalSizeVertical[0]<<" -D V_GROUPSIZE_Y="<<m_LocalSizeVertical[1]
	<<" -D V_RESULT_STEPS="<<m_StepsVertical;

	m_Program = CLUtil::BuildCLProgramFromMemory(Device, Context, programCode, compileOptions.str());
	if(m_Program == nullptr) return false;

	m_SobelProgram = CLUtil::BuildCLProgramFromMemory(Device, Context, sobelCode, compileOptions.str());
	if(m_Program == nullptr) return false;

	m_NonMaxProgram = CLUtil::BuildCLProgramFromMemory(Device, Context, nonMaxCode, compileOptions.str());
	if(m_Program == nullptr) return false;
	
    m_HystProgram = CLUtil::BuildCLProgramFromMemory(Device, Context, hystCode, compileOptions.str());
	if(m_Program == nullptr) return false;
    
    return InitKernels();
}

bool CConvolutionSeparableTask::InitKernels()
{

	cl_int clError;
	
	//create kernel(s)
	m_HorizontalKernel = clCreateKernel(m_Program, "ConvHorizontal", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create horizontal kernel.");
	
	m_VerticalKernel = clCreateKernel(m_Program, "ConvVertical", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create vertical kernel.");

    m_SobelKernel = clCreateKernel(m_SobelProgram, "Sobel", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Sobel kernel.");
	
    m_NonMaxKernel = clCreateKernel(m_NonMaxProgram, "NonMaxSuppression", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create NMS kernel.");
    
    m_HystKernel = clCreateKernel(m_HystProgram, "Hysteresis", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Hyst kernel.");
    
    m_HystKernelComp = clCreateKernel(m_HystProgram, "CompleteHysteresis", &clError);
	V_RETURN_FALSE_CL(clError, "Failed to create Hyst Comp kernel.");

    //bind kernel attributes
	//the resulting image will be in buffer 1
	clError = clSetKernelArg(m_HorizontalKernel, 2, sizeof(cl_mem), (void*)&m_dKernelHorizontal);
	clError |= clSetKernelArg(m_HorizontalKernel, 3, sizeof(cl_uint), (void*)&m_Width);
	clError |= clSetKernelArg(m_HorizontalKernel, 4, sizeof(cl_uint), (void*)&m_Pitch);
	V_RETURN_FALSE_CL(clError, "Error setting horizontal kernel arguments");
		
	//the resulting image will be in buffer 0
	clError = clSetKernelArg(m_VerticalKernel, 2, sizeof(cl_mem), (void*)&m_dKernelVertical);
	clError |= clSetKernelArg(m_VerticalKernel, 3, sizeof(cl_uint), (void*)&m_Height);
	clError |= clSetKernelArg(m_VerticalKernel, 4, sizeof(cl_uint), (void*)&m_Pitch);
	V_RETURN_FALSE_CL(clError, "Error setting vertical kernel arguments");

	//the resulting image will be in buffer 0
	clError |= clSetKernelArg(m_SobelKernel, 3, sizeof(cl_uint), (void*)&m_Width);
	clError |= clSetKernelArg(m_SobelKernel, 4, sizeof(cl_uint), (void*)&m_Height);
	clError |= clSetKernelArg(m_SobelKernel, 5, sizeof(cl_uint), (void*)&m_Pitch);
	V_RETURN_FALSE_CL(clError, "Error setting sobel kernel arguments");
	
	//the resulting image will be in buffer 0
	clError |= clSetKernelArg(m_NonMaxKernel, 3, sizeof(cl_uint), (void*)&m_Width);
	clError |= clSetKernelArg(m_NonMaxKernel, 4, sizeof(cl_uint), (void*)&m_Height);
	clError |= clSetKernelArg(m_NonMaxKernel, 5, sizeof(cl_uint), (void*)&m_Pitch);
	V_RETURN_FALSE_CL(clError, "Error setting non max kernel arguments");
    
	//the resulting image will be in buffer 0
	clError |= clSetKernelArg(m_HystKernel, 2, sizeof(cl_uint), (void*)&m_Width);
	clError |= clSetKernelArg(m_HystKernel, 3, sizeof(cl_uint), (void*)&m_Height);
	clError |= clSetKernelArg(m_HystKernel, 4, sizeof(cl_uint), (void*)&m_Pitch);
	V_RETURN_FALSE_CL(clError, "Error setting Optimized hyst kernel arguments");
    
	//the resulting image will be in buffer 0
	clError |= clSetKernelArg(m_HystKernelComp, 2, sizeof(cl_uint), (void*)&m_Width);
	clError |= clSetKernelArg(m_HystKernelComp, 3, sizeof(cl_uint), (void*)&m_Height);
	clError |= clSetKernelArg(m_HystKernelComp, 4, sizeof(cl_uint), (void*)&m_Pitch);
	V_RETURN_FALSE_CL(clError, "Error setting Optimized hyst kernel arguments");
    
    return true;
}

void CConvolutionSeparableTask::ReleaseResources()
{
	SAFE_DELETE_ARRAY( m_hCPUWorkingBuffer );

    SAFE_RELEASE_MEMOBJECT(m_dSobelAngleBuffer);
	SAFE_RELEASE_MEMOBJECT(m_dGPUWorkingBuffer);
	SAFE_RELEASE_MEMOBJECT(m_dKernelHorizontal);
	SAFE_RELEASE_MEMOBJECT(m_dKernelVertical);

	SAFE_RELEASE_KERNEL(m_HorizontalKernel);
	SAFE_RELEASE_KERNEL(m_VerticalKernel);
    SAFE_RELEASE_KERNEL(m_SobelKernel);
    SAFE_RELEASE_KERNEL(m_NonMaxKernel);
    SAFE_RELEASE_KERNEL(m_HystKernel);
	
    SAFE_RELEASE_PROGRAM(m_Program);
    SAFE_RELEASE_PROGRAM(m_SobelProgram);
    SAFE_RELEASE_PROGRAM(m_NonMaxProgram);
    SAFE_RELEASE_PROGRAM(m_HystProgram);
}

void CConvolutionSeparableTask::ComputeGPU(cl_context Context, cl_command_queue CommandQueue, size_t LocalWorkSize[3])
{
    CTimer timer;
	size_t dataSize = m_Pitch * m_Height * sizeof(cl_float);
	
    cl_int clErr;

    timer.Start();
    // Running the gaussian Kernel to smooth out the image
	clErr  = clSetKernelArg(m_HorizontalKernel, 0, sizeof(cl_mem), (void*)&m_dGPUWorkingBuffer);
	clErr |= clSetKernelArg(m_HorizontalKernel, 1, sizeof(cl_mem), (void*)&m_dSourceChannels[0]);
	V_RETURN_CL(clErr, "Error setting horizontal kernel arguments");

	clErr  = clSetKernelArg(m_VerticalKernel, 0, sizeof(cl_mem), (void*)&m_dResultChannels[0]);
	clErr |= clSetKernelArg(m_VerticalKernel, 1, sizeof(cl_mem), (void*)&m_dGPUWorkingBuffer);
	V_RETURN_CL(clErr, "Error setting vertical kernel arguments");

	size_t globalWorkSizeH[2] = {
		CLUtil::GetGlobalWorkSize(m_Width / m_StepsHorizontal, m_LocalSizeHorizontal[0]),
		CLUtil::GetGlobalWorkSize(m_Height, m_LocalSizeHorizontal[1])
	};	

    clErr |= clEnqueueNDRangeKernel(CommandQueue, m_HorizontalKernel, 2, NULL, globalWorkSizeH, m_LocalSizeHorizontal, 0, NULL, NULL);

	size_t globalWorkSizeV[2] = {
		CLUtil::GetGlobalWorkSize(m_Width, m_LocalSizeVertical[0]),
		CLUtil::GetGlobalWorkSize(m_Height / m_StepsVertical, m_LocalSizeVertical[1])
	};
    
    clErr |= clEnqueueNDRangeKernel(CommandQueue, m_VerticalKernel, 2, NULL, globalWorkSizeV, m_LocalSizeVertical, 0, NULL, NULL);

    // Run the sobel kernels to get the rough edges from the image
    
    size_t globalWorkSizeS[2] = {
		CLUtil::GetGlobalWorkSize(m_Width, m_LocalSizeVertical[0]),
		CLUtil::GetGlobalWorkSize(m_Height, m_LocalSizeVertical[1])
    };

    clErr = clSetKernelArg(m_SobelKernel, 0, sizeof(cl_mem), (void*)&m_dGPUWorkingBuffer);
    clErr |= clSetKernelArg(m_SobelKernel, 1, sizeof(cl_mem), (void*)&m_dSobelAngleBuffer);
    clErr |= clSetKernelArg(m_SobelKernel, 2, sizeof(cl_mem), (void*)&m_dResultChannels[0]);

    V_RETURN_CL(clErr, "Error binding sobel variables");

    clErr |= clEnqueueNDRangeKernel(CommandQueue, m_SobelKernel, 2, NULL, globalWorkSizeS, m_LocalSizeVertical, 0, NULL, NULL);
    
    V_RETURN_CL( clEnqueueReadBuffer(CommandQueue, m_dGPUWorkingBuffer, CL_TRUE, 0, dataSize,
                                m_hGPUResultChannels[0], 0, NULL, NULL), "Error reading back results from the device!" );
	
	SaveImage("Images/GPUResult_Sobel.pfm", m_hGPUResultChannels);

    // Run the non maximum suppression to thin the edges

    //out
    clErr = clSetKernelArg(m_NonMaxKernel, 0, sizeof(cl_mem), (void*)&m_dResultChannels[0]);
    //in
    clErr |= clSetKernelArg(m_NonMaxKernel, 1, sizeof(cl_mem), (void*)&m_dSobelAngleBuffer);
    //in
    clErr |= clSetKernelArg(m_NonMaxKernel, 2, sizeof(cl_mem), (void*)&m_dGPUWorkingBuffer);

    V_RETURN_CL(clErr, "Error binding sobel variables");

    clErr |= clEnqueueNDRangeKernel(CommandQueue, m_NonMaxKernel, 2, NULL, globalWorkSizeS, m_LocalSizeVertical, 0, NULL, NULL);
    
    V_RETURN_CL( clEnqueueReadBuffer(CommandQueue, m_dResultChannels[0], CL_TRUE, 0, dataSize,
                                m_hGPUResultChannels[0], 0, NULL, NULL), "Error reading back results from the device!" );
	
	SaveImage("Images/GPUResult_Suppressed.pfm", m_hGPUResultChannels);

    // Threshold the values to get the definitive edges
    
#if 0
    clErr = clSetKernelArg(m_HystKernel, 0, sizeof(cl_mem), (void*)&m_dGPUWorkingBuffer);
    clErr |= clSetKernelArg(m_HystKernel, 1, sizeof(cl_mem), (void*)&m_dResultChannels[0]);
    V_RETURN_CL(clErr, "Error binding Hysteresis variables");

    clErr |= clEnqueueNDRangeKernel(CommandQueue, m_HystKernel, 2, NULL, globalWorkSizeS, m_LocalSizeVertical, 0, NULL, NULL);
#else

    for (int i = 0; i < 2; i++)
    {
        clErr = clSetKernelArg(m_HystKernelComp, 0, sizeof(cl_mem), (void*)&m_dGPUWorkingBuffer);
        clErr |= clSetKernelArg(m_HystKernelComp, 1, sizeof(cl_mem), (void*)&m_dResultChannels[0]);
        V_RETURN_CL(clErr, "Error binding sobel variables");

        clErr |= clEnqueueNDRangeKernel(CommandQueue, m_HystKernelComp, 2, NULL, globalWorkSizeS, m_LocalSizeVertical, 0, NULL, NULL);
    }
#endif

    //copy the results back to the CPU
    //(this time the data is in the same buffer as the input was, because of the 2 convolution passes)
    timer.Stop();
    printf("\n Elapsed time is: %lf ms\n", timer.GetElapsedMilliseconds()); 
    V_RETURN_CL( clEnqueueReadBuffer(CommandQueue, m_dGPUWorkingBuffer, CL_TRUE, 0, dataSize,
                                m_hGPUResultChannels[0], 0, NULL, NULL), "Error reading back results from the device!" );
	
	SaveImage("Images/GPUResult_Thresholded_final.pfm", m_hGPUResultChannels);
}
///////////////////////////////////////////////////////////////////////////////
