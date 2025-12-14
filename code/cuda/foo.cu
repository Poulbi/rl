#define FORCE_THREADS_COUNT 1
#include "base/base.h"

#include "cu.h"

CU_kernel void 
MyKernel(int *d, int *a, int *b)
{
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    d[idx] = a[idx] * b[idx];
}

ENTRY_POINT(EntryPoint)
{
    int numBlocks;
    int blockSize = 32;
    
    int device;
    cudaDeviceProp prop;
    int activeWarps;
    int maxWarps;
    
    s32 IntendedSharedMemorySize = 0;
    
    CU_Check(cudaSetDevice(0));
    CU_Check(cudaGetDeviceProperties(&prop, 0));
    
    s32 BlocksCount;
    CU_Check(cudaOccupancyMaxActiveBlocksPerMultiprocessor(&BlocksCount, 
                                                           MyKernel, 
                                                           blockSize, 
                                                           IntendedSharedMemorySize));
    
    activeWarps = BlocksCount * blockSize / prop.warpSize;
    maxWarps = prop.maxThreadsPerMultiProcessor / prop.warpSize;
    
    OS_PrintFormat("Occupancy: %.2f%\n",
                   (double)activeWarps / maxWarps * 100);
    
    return 0;
}


