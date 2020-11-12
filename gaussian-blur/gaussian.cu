// Josh Shepherd 1700471
#include <stdio.h>
#include <stdlib.h>
#include "lodepng.h"

/**
Applies Gaussian Blur using CUDA. 
*/
__global__
void gaussianBlur(unsigned char* deviceInput, unsigned char* deviceOutput) {
	/// Get unique id of for the thread
	int uid = blockDim.x * blockIdx.x + threadIdx;

	printf("Thread ID: '%d'\n", uid);
}

/**
	Gaussian blur using CUDA threads. Takes two arguments, 
	1: Path name the input png file
	2: Path name of the gaussian blurred png file
*/
int main (int argc, char* argv[]) {
	// Get file name of png
	char* fileName = "img.png";
    if (argc > 1)
        fileName = argv[1];
    // Get gaussian blur output file name
    char* outputFileName = "output.png";
    if (argc > 2)
    	outputFileName = argv[2];

	printf("Blurring image '%s'\n", fileName);
   
	/// Initially load PNG file using lodepng
	unsigned int width, height;
	unsigned char* pngValues;
	lodepng_decode32_file(&pngValues, &width, &height, fileName);

	// Check if image loaded is valid
	if (width <= 0 || height <= 0) {
        printf("Unable to decode image. Validate file and try again");
        exit(-1);
    }
    
    /// Malloc device original png values
	unsigned char* d_originalVals;
    int originalValsLength = width * height;	
    cudaMalloc((void**) &d_originalVals, originalValsLength);
    /// Transfer from CPU to GPU
    cudaMemcpy(d_originalVals, pngValues, originalValsLength, cudaMemcpyHostToDevice);
    
    // cuda malloc the final blurred vals array using width * height
    unsigned char* d_blurredVals;
    cudaMalloc((void**) &d_blurredVals, blurredArrayLength);
   

    /// Launch CUDA to gaussian blur original vals to blurred vals
    gaussianBlur<<< dim3(1, 1, 1), dim3(1, 1, 1) >>>(d_originalVals, d_blurredVals);
   	cudaThreadSynchronize();
    
    
    /// Copy final CUDA blurred img vals to CPU
    unsigned char* blurredImgVals;
    cudaMemcpy(blurredImgVals, d_blurredVals, originalValsLength, cudaMemcpyDeviceToHost);
    
    /// Save blurred values to png file
    unsigned char* threadImgValues;
    lodepng_encode32_file(outFileName, threadImgValues, width, height);
    printf("Successfully blurred the image into ./'%s'\n", outFileName);
    
    /// Free any malloc & CUDA malloc
    free(pngValues);
    free(blurredImgVals);
    cudaFree(d_blurredVals);
    cudaFree(d_originalVals);
}
