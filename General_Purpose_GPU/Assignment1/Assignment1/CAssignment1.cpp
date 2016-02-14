/******************************************************************************
GPU Computing / GPGPU Praktikum source code.

******************************************************************************/

#include "CAssignment1.h"

#include "CSimpleArraysTask.h"
#include "CMatrixRotateTask.h"

#include <iostream>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CAssignment1

bool CAssignment1::DoCompute()
{
	// Task 1: simple array addition.
	cout << "Running vector addition example..." << endl << endl;
	{
		//256 1 1
		size_t localWorkSize[3] = {16, 1, 1};
		
		//1048576
		CSimpleArraysTask task(1048576);
		RunComputeTask(task, localWorkSize);
	}
	{
		//512
		size_t LocalWorkSize[3] = {32, 1, 1};
		
		//1048576
		CSimpleArraysTask task(1048576);
		RunComputeTask(task, LocalWorkSize);
	}
	// Task 2: matrix rotation.
	std::cout << "Running matrix rotation example..." << std::endl << std::endl;
	{
		// 32 16 1
		size_t LocalWorkSize[3] = {16, 16, 1};
		// 2048 1025
		CMatrixRotateTask task(1025, 1025);
		RunComputeTask(task, LocalWorkSize);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
