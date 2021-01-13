// Josh Shepherd 1700471
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lodepng.h"

struct RGBA {
	int r;
	int g;
	int b;
	int a;
};

/// Maximum for an RGBA value
const int MAX_VALUE = 255;
const int ERROR_EXIT_VALUE = -1;

void PrintRGBAArray(unsigned char* rgbaArray, int height, int width);

/**
	Averages all values inside vals array.
	vals: RGBA values of the surrounding pixels
	valsLength: length of the RGBA vals array
	totalValidValues: amount of valid RGBA values inside vals
*/
__device__
struct RGBA GetAverage(struct RGBA* vals, int valsLength, int totalValidValues)
{
	///	Init total values for all RGB (ignore alpha for average)
	double totalR = 0.0, totalG = 0.0, totalB = 0.0;
	
	/// Total all rgb values inside vals array
	for (int i = 0; i < valsLength; i++) {
		totalR += vals[i].r;
		totalG += vals[i].g;
		totalB += vals[i].b;
	}
	
	/// Divide totals by amount of valid pixel values
	struct RGBA averagedVals;
	averagedVals.r = totalR / totalValidValues;
	averagedVals.g = totalG / totalValidValues;
	averagedVals.b = totalB / totalValidValues;
	averagedVals.a = MAX_VALUE;
	
	return averagedVals;
}

/**
	Gets the RGBA values inside of an array of pixel values from the index
*/
__device__
struct RGBA GetRGBAValuesAtIndex(unsigned char* imgValues, int index) {
	struct RGBA values;
	values.r = imgValues[index];
	values.g = imgValues[index + 1];
	values.b = imgValues[index + 2];
	values.a = imgValues[index + 3];
	return values;
}

/**
	Applies Gaussian Blur using CUDA. 
*/
__global__
void gaussianBlur(unsigned char* originalVals, unsigned char* blurredChars, int width, int height) {
	/// Get unique id of for the thread
	int uid = blockDim.x * blockIdx.x + threadIdx.x;
	
	/// index of which pixel this thread will focus on to blur
	int pixelY = uid % width;
	int pixelX = uid / width;

	/// Check the values are valid
	if (pixelX > width || pixelY > height) {
		printf("Unable to correctly determine pixelX and pixelY in thread '%d'\n", uid);
		return;
	}

	//printf("Thread ID: '%d' pixelX: '%d' pixelY: '%d'\n", uid, pixelX, pixelY);
	
	/// Store all RGBA values around the focus pixel
	struct RGBA* thisValues = (struct RGBA*) malloc( sizeof(struct RGBA) * 9 );

	int arrayStartIndex = uid * 4;
	if (arrayStartIndex < 0 || arrayStartIndex >= width * height * 4) {
		printf("Unable to determine arrayStartIndex in thread '%d'\n", uid);
		return;
	}
	
	int validValuesCount = 0;
	
	/// Get the middle focus pixel RGBA values
	thisValues[4] = GetRGBAValuesAtIndex(originalVals, arrayStartIndex);
	validValuesCount++;
	
	//printf("Thread '%d' focus pixel x:'%d' y:'%d' r: %d g: %d b: %d a: %d \n", uid, pixelX, pixelY, thisValues[4].r, thisValues[4].g, thisValues[4].b, thisValues[4].a);
	
	/// Determine if current pixel is at any walls so as to get valid values
	bool atLeftWall = pixelY == 0;
	bool atTopWall = pixelX == 0;
	bool atRightWall = pixelY > 0 && pixelY % (width - 1) * 4 == 0;
	bool atBtmWall = pixelX >= height - 1;
	
	
	int colArrayIndex = pixelY * 4;
	
	/// Debug: Print isAtWall bools
	//printf("Thread '%d' atLeftWall: '%d' atTopWall: '%d' atRightWall: '%d' atBtmWall: '%d' \n", uid, atLeftWall, atTopWall, atRightWall, atBtmWall);
		
	if ( !atLeftWall ) {
	
		/// Middle Left value
		int mlIndex = (pixelX * width * 4) + (colArrayIndex - (4 * 1));
		thisValues[3] = GetRGBAValuesAtIndex( originalVals, mlIndex );
		validValuesCount++;
		
		/// Top Left value
		if ( !atTopWall ) {  
			int tlIndex = ((pixelX - 1) * width * 4) + (colArrayIndex - (4 * 1));
			thisValues[0] = GetRGBAValuesAtIndex( originalVals, tlIndex );
			validValuesCount++;
		}
		
		/// Bottom Left value
		if ( !atBtmWall ) {
			int blIndex = ((pixelX + 1) * width * 4) + (colArrayIndex - (4 * 1));
			thisValues[6] = GetRGBAValuesAtIndex( originalVals, blIndex );
			validValuesCount++;
		}
	}
	
	if ( !atRightWall) {
		
		/// Middle Right value
		int mrIndex = (pixelX * width * 4) + (colArrayIndex + (4 * 1));
		thisValues[5] = GetRGBAValuesAtIndex( originalVals, mrIndex );
		validValuesCount++;
		
		if ( !atTopWall ) {
			/// Top Right value
			int trIndex = ((pixelX - 1) * width * 4) + (colArrayIndex + (4 * 1));
			thisValues[2] = GetRGBAValuesAtIndex( originalVals, trIndex );
			validValuesCount++;
		}
		
		if ( !atBtmWall ) {
			/// Bottom Right value
			int brIndex = ((pixelX + 1) * width * 4) + (colArrayIndex + (4 * 1));
			thisValues[8] = GetRGBAValuesAtIndex( originalVals, brIndex );
			validValuesCount++;
		}
	}
	
	if ( !atTopWall ) {
		/// Top Middle value
		int tmIndex = ((pixelX - 1) * width * 4) + colArrayIndex;
		thisValues[1] = GetRGBAValuesAtIndex( originalVals, tmIndex );
		validValuesCount++;
	}
	
	if ( !atBtmWall ) {
		/// Bottom Middle value
		int bmIndex = ((pixelX + 1) * width * 4) + colArrayIndex;
		thisValues[7] = GetRGBAValuesAtIndex( originalVals, bmIndex );
		validValuesCount++;
	}
	
	//printf("Thread '%d' at pixelX: '%d' pixelY: '%d'  got '%d' values\n", uid, pixelX, pixelY, validValuesCount);
	
	/// Get blurred values from surrounding RGBA values
	struct RGBA blurredVals = GetAverage(thisValues, 9, validValuesCount);
	
	/// Insert RGBA values into blurredChars array
	blurredChars[arrayStartIndex] = blurredVals.r;
	blurredChars[arrayStartIndex + 1] = blurredVals.g;
	blurredChars[arrayStartIndex + 2] = blurredVals.b;
	blurredChars[arrayStartIndex + 3] = blurredVals.a;
	
	// Free malloc'd thisValues array
	free(thisValues);
}

/**
	Gaussian blur using CUDA threads. Takes two arguments, 
	1: Path name the input png file
	2: Path name of the output blurred png file
*/
int main (int argc, char* argv[]) {
	printf("Josh Shepherd - 1700471\n\n");
	
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
	unsigned int lodepng_error;
	unsigned char* cpuPngValues = (unsigned char*) malloc( sizeof(unsigned char) * width * height * 4 );
	lodepng_error = lodepng_decode32_file(&cpuPngValues, &width, &height, fileName);

	/// Check for any decoding errors
	if (lodepng_error) {
		printf("Error decoding png file: '%u' '%s'\n", lodepng_error, lodepng_error_text(lodepng_error));
		exit(ERROR_EXIT_VALUE);
	}

	int totalImgPixelValues = width * height * 4;
	//printf("Total Img Pixel Values: width (%d) * height (%d) * 4 = '%d'\n", width, height, totalImgPixelValues);

	// Check if image loaded is valid
	if (width <= 0 || height <= 0) {
        printf("Unable to decode image. Validate file and try again\n");
        exit(ERROR_EXIT_VALUE);
    }
    
    /// Malloc device original png values
	unsigned char* gpuOriginalVals;
    cudaMalloc((void**) &gpuOriginalVals, sizeof(unsigned char) * totalImgPixelValues);
    cudaMemcpy(gpuOriginalVals, cpuPngValues, sizeof(unsigned char) * totalImgPixelValues, cudaMemcpyHostToDevice);
    
    // cuda malloc the final blurred vals array using width * height
    unsigned char* gpuBlurredVals;
    cudaMalloc((void**) &gpuBlurredVals, sizeof(unsigned char) * totalImgPixelValues );
	

    /// Launch CUDA to gaussian blur original vals to blurred vals
    gaussianBlur<<< dim3(width, 1, 1), dim3(height, 1, 1) >>>(gpuOriginalVals, gpuBlurredVals, width, height);
   	cudaDeviceSynchronize();
   	
   	
    printf("Finished all CUDA threads\n");
    
    /// Copy final CUDA blurred img vals to CPU
    unsigned char* cpuBlurredImgVals = (unsigned char*) malloc( sizeof(unsigned char) * totalImgPixelValues );
    cudaMemcpy(cpuBlurredImgVals, gpuBlurredVals, sizeof(unsigned char) * totalImgPixelValues, cudaMemcpyDeviceToHost);
    //PrintRGBAArray(cpuBlurredImgVals, height, width);

    /// Save blurred values to png file
    lodepng_error = lodepng_encode32_file(outputFileName, cpuBlurredImgVals, width, height);
     
 	/// Check for any encoding errors
	if (lodepng_error) {
		printf("Error encoding png file: '%u' '%s'\n", lodepng_error, lodepng_error_text(lodepng_error));
		exit(ERROR_EXIT_VALUE);
	}
 
	printf("Successfully blurred the image into ./'%s'\n", outputFileName);
    
    /// Free any malloc & CUDA malloc
    free(cpuPngValues);
    free(cpuBlurredImgVals);
    cudaFree(gpuOriginalVals);
    cudaFree(gpuBlurredVals);
}

/**
	Prints all array of RGBA values inside an array
*/
void PrintRGBAArray(unsigned char* rgbaArray, int height, int width)
{
    for( int row = 0; row < height; row++ ) {
        for ( int col = 0; col < width*4; col += 4 ) {
            printf("Row: '%d' Col: '%d' R:%d, G:%d, B:%d, A:%d\n", row, col / 4, rgbaArray[row*width*4+col], rgbaArray[row*width*4+col+1], rgbaArray[row*width*4+col+2], rgbaArray[row*width*4+col+3]);
        }
    }
    printf("\n");
}

