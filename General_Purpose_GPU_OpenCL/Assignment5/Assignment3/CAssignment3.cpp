/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CAssignment3.h"

#include "CConvolutionSeparableTask.h"
#include <iostream>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CAssignment3

bool CAssignment3::DoCompute()
{
	cout<<"########################################"<<endl;
	cout<<"GPU Computing assignment 5"<<endl<<endl;

	cout<<"My freestyle Assignment, making canny edge detection, based on the third assignment."<<endl;

	cout<<endl<<"########################################"<<endl;
	cout<<"Task: Canny Edge detection"<<endl<<endl;
	{
		size_t HGroupSize[2] = {32, 16};
		size_t VGroupSize[2] = {32, 16};

		{
			// Gaussian blur
			float ConvKernel[7] = {
				0.000817774f, 0.0286433f, 0.235018f, 0.471041f, 0.235018f, 0.0286433f, 0.000817774f
			};
			CConvolutionSeparableTask convTask("gauss_3x3", "Images/input.pfm", HGroupSize, VGroupSize,
				4, 4, 3, ConvKernel, ConvKernel);
			RunComputeTask(convTask, HGroupSize);
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
