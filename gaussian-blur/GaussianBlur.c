// Josh Shepherd 1700471
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include "lodepng.h"

struct RGBA {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct ThreadArgs {
    // Original image RGBA values
    unsigned char* values;
    // Index to start at in original image values
    int start;
    // Index to end at in original image values
    int end;
    // Width & Height of original image
    unsigned int width;
    unsigned int height;
    // Modified new RGVA values
    unsigned char* rtnValues;
};

// Single Threaded method of applying Gaussian Blur
unsigned char* GaussianBlur(unsigned char* imgValues, int width, int height);
// Multithreaded way of applying GaussianBlur
void* GaussianBlurThread(void* threadArgsParam);
// Gets the next 4 values from the index and returning them as an RGBA struct
struct RGBA GetSetOfValues(unsigned char* imgValues, int fromIndex);
// Gets the average RGBA value from a 3x3 matrix of the current pixel
struct RGBA GetAverage(struct RGBA* values, int length, int validValues);
// Prints out a single dimension array with RGBA formatting
void PrintRGBAArray(unsigned char* rgbaArray, int width, int length);

int main (int argc, char *argv[]) {

    char* fileName = "c.png";
    if (argc > 1)
        fileName = argv[1];

    int threadCount = 10;
    if (argc > 2)
        threadCount = atoi(argv[2]);

    printf("Blurring image '%s'\n", fileName);

	/// Initially load PNG file using lodepng
    unsigned int width, height;
    unsigned char* values;
    lodepng_decode32_file(&values, &width, &height, fileName);

    if (width <= 0 || height <= 0) {
        printf("Unable to decode image. Validate file and try again");
        exit(-1);
    }

    //printf("Original Values:\n");
    //PrintRGBAArray(values, width, height);

    // Create arrays for storing threads and their arguments
    struct ThreadArgs* threadArgs = malloc( sizeof(struct ThreadArgs) * threadCount );
    pthread_t* threads = malloc( sizeof(pthread_t) * threadCount );

    // Work out the amount of pixels each thread needs to work
    // and create args/threads
    double amtPixels = (width * height) / threadCount;
    double remainderPixels = (width * height) % threadCount;
    for (int i = 0; i < threadCount; i++) {
        threadArgs[i].values = values;
        threadArgs[i].start = (amtPixels * 4) * i;
        threadArgs[i].end = ((amtPixels * 4) * i) + (amtPixels * 4);
        // Add remaining pixels to last thread
        if (remainderPixels > 0 && i == threadCount - 1) {
            threadArgs[i].end += (remainderPixels * 4);
        }
        threadArgs[i].width = width;
        threadArgs[i].height = height;   

        pthread_create(&threads[i], NULL, GaussianBlurThread, &threadArgs[i]);
    }

    // Make array same size as original image for storing the blurred values
    unsigned char* threadImgValues = malloc ( sizeof(unsigned char) * width * height * 4);
    void* current;
    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], &current);
        
        // Get the returned values and insert them to threadImgValues at their appropriate index
        struct ThreadArgs args = *(struct ThreadArgs*)current;
        for (int i = args.start; i < args.end; i++) {
            threadImgValues[i] = args.rtnValues[i];
        }
        // free the malloc'd values used to pass the data through
        free(args.rtnValues);
    }

    // Output the data to a file
    char* outFileName = "Output.png";
    lodepng_encode32_file(outFileName, threadImgValues, width, height);
    printf("Successfully blurred the image into ./'%s'\n", outFileName);

    printf("Gaussian Blurred Values:\n");
    PrintRGBAArray(threadImgValues, width, height);

    // Free any malloc'd variables before finishing
    free(values);
    free(threadImgValues);
    free(threadArgs);
    free(threads);
}

void* GaussianBlurThread(void* threadArgsParam)
{
    struct ThreadArgs tArgs = *(struct ThreadArgs*)threadArgsParam;
    int width = tArgs.width;
    int height = tArgs.height;
    unsigned char* imgValues = tArgs.values;
    int tStartIndex = tArgs.start;
    int tEndIndex = tArgs.end;

    unsigned char* blurred = malloc( sizeof(char) * width * height * 4 );

    // Calculate which pixels to start and stop at
    int startRow = tStartIndex / (width * 4);
    int endRow = tEndIndex / (width * 4);
    int startCol = (tStartIndex % (width * 4)) / 4;
    int endCol = (tEndIndex % (width * 4)) / 4;

    // If tEndIndex is final value, set endCol to width as to not mess with col for loop
    if ( tEndIndex == width * height * 4) {
        endCol = width * 4;
    }

    for( int row = startRow; row <= endRow; row++ ) {
        for ( int col = 0; col < width * 4; col += 4 ) {
            // Ignore pixels before startCol if on startRow
            if (row == startRow && col < startCol) {
                continue;
            }
            // If on final processing row for this section, ignore any pixles after endCol
            // (* 4 since columns are in pixels, array is in RGBA values)
            else if (row == endRow && col > (endCol * 4)) {
                continue;
            }

            // Make array to store potentially 9 sets of RGBA values
            struct RGBA* thisValues = malloc( sizeof(struct RGBA) * 9 );
            for (int i = 0; i < 9; i++) {
                thisValues[i].a = 0;
                thisValues[i].r = 0;
                thisValues[i].g = 0;
                thisValues[i].b = 0;
            }

            int startIndex = (row * width * 4) + col;
            // If on last row but calculated index as more than array, ignore
            // occures on last thread iterating over last row
            if (startIndex < 0 || startIndex >= width * height * 4) {
                continue;
            }

            // Keep track of how many pixels are around and used to blur this pixel (including itself)
            int valCount = 0;

            // Middle Center Value - Target of Gaussian Blur
            thisValues[4] = GetSetOfValues(imgValues, startIndex);
            valCount++;

            // Is the Center Pixel at any of the walls
            bool isAtLeftWall = col == 0;
            bool isAtTopWall = row == 0;
            bool isAtRightWall =  col > 0 && col % (width-1) * 4 == 0;
            bool isAtBtmWall = row >= height - 1;
            if ( !isAtLeftWall ) {

                // Left Value
                thisValues[3] = GetSetOfValues(imgValues, (row * width * 4) + (col - (4 * 1)));
                valCount++;
                // Top Left
                if ( !isAtTopWall ) {
                    thisValues[0] = GetSetOfValues (imgValues, ((row - 1) * width * 4) + (col - (4 * 1)) );
                    valCount++;
                }
                // Bottom left 
                if ( !isAtBtmWall ) {
                    thisValues[6] = GetSetOfValues(imgValues, ((row + 1) * width * 4) + (col - (4 * 1)) );
                    valCount++;
                }
            }

            if ( !isAtRightWall) {
                
                // Right Value
                thisValues[5] = GetSetOfValues(imgValues, (row * width * 4) + (col + (4 * 1)));
                valCount++;

                // Top Right 
                if ( !isAtTopWall ) { 
                    thisValues[2] = GetSetOfValues(imgValues, ((row - 1) * width * 4) + (col + (4 * 1)));
                    valCount++;
                }
                // Btm Right
                if ( !isAtBtmWall ) {
                    thisValues[8] = GetSetOfValues(imgValues, ((row + 1) * width * 4) + (col + (4 * 1)));
                    valCount++;
                }
            } 

            if ( !isAtTopWall ) {
                // Top Value
                thisValues[1] = GetSetOfValues(imgValues, ((row - 1) * width * 4) + (col));
                valCount++;
            }

            if ( !isAtBtmWall ) {
                // Btm Value
                thisValues[7] = GetSetOfValues(imgValues, ((row + 1) * width * 4) + (col));
                valCount++;
            }
            
            struct RGBA blurredValues = GetAverage(thisValues, 9, valCount);
            blurred[startIndex] = blurredValues.r;
            blurred[startIndex + 1] = blurredValues.g;
            blurred[startIndex + 2] = blurredValues.b;
            blurred[startIndex + 3] = blurredValues.a;

            free(thisValues);
        }
    }

    tArgs.rtnValues = blurred;
    *((struct ThreadArgs*)threadArgsParam) = tArgs;
    pthread_exit( threadArgsParam );
}

struct RGBA GetSetOfValues(unsigned char *imgValues, int fromIndex) {
    struct RGBA vals;
    vals.r = imgValues[fromIndex];
    vals.g = imgValues[fromIndex + 1];
    vals.b = imgValues[fromIndex + 2];
    vals.a = imgValues[fromIndex + 3];
    return vals;
}

struct RGBA GetAverage(struct RGBA* values, int length, int validValues)
{
    double totalR = 0.0;
    double totalG = 0.0;
    double totalB = 0.0;
    //double totalA = 0.0;
    for ( int i = 0; i < length; i++ ) {
        totalR += values[i].r;
        totalG += values[i].g;
        totalB += values[i].b;
        //totalA += values[i].a;
    }

    struct RGBA final;
    final.r = totalR / validValues;
    final.g = totalG / validValues;
    final.b = totalB / validValues;
    final.a = (unsigned char)255;
    //final.a = totalA / validValues;

    return final;
}

void PrintRGBAArray(unsigned char* rgbaArray, int height, int width)
{
    for( int row = 0; row < height; row++ ) {
        for ( int col = 0; col < width*4; col += 4 ) {
            printf("R:%d, G:%d, B:%d, A:%d", rgbaArray[row*width*4+col], rgbaArray[row*width*4+col+1], rgbaArray[row*width*4+col+2], rgbaArray[row*width*4+col+3]);
            printf(" | ");
        }
    }
    printf("\n");
}
