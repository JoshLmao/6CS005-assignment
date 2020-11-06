#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256

struct MatrixSize {
    int x;
    int y;
};

struct Matrix {
    struct MatrixSize size;
    double* values;
};

/*
* Checks if the char array line is the matrix definition line and returns the size is true
* (AxB where A and B is the matrix size)
*/
struct MatrixSize* charArrayIsMatrixSize (char * chars, int size) {
    struct MatrixSize* mtxSize = malloc ( sizeof(struct MatrixSize ) );
    int splitCount = 0;

    // make copy of chars and test it
    // storing in temp to not add delimiters in original char array
    char temp[BUFFER_SIZE];
    strcpy(temp, chars);

    char* split = strtok(temp, ",");
    while (split != NULL) {
        double parsed = atof(split);
        if ( parsed > 0 ) 
        {
            if (splitCount == 0) {
                mtxSize->x = parsed;
            } else if (splitCount == 1) {
                mtxSize->y = parsed;
            } else {
                // More commas than expected. Is a matrix value line
                return NULL;
            }

            // increment split count when value is found
            splitCount++;
        }

        // Move split onto next split index
        split = strtok(NULL, ",");
    }

    return mtxSize;
}

/*
* Checks if the first character in the array is blank or new line
*/
int charArrayIsBlank (char * chars, int size) {
    return chars[0] == '\n';
}

/*
* Parses a matrix line of doubles separated with commas
*/
double* parseLineOfDoubles(char* lineChars, struct MatrixSize mtxSize) {
    double* lineVals = malloc (sizeof(double) * mtxSize.y);
    int lineYCount = 0;

    const char splitter[2] = ",";
    char* split = strtok(lineChars, splitter);
    while (split) {
        double parsed = atof(split);
        if ( parsed > 0 ) {
            lineVals[lineYCount] = parsed;
            lineYCount += 1;
        }

        // Move split onto next split index
        split = strtok(NULL, splitter);
    }

    return lineVals;
}

/*
* Gets the total matrices in the specified file format
*/
int getFileMatrixCount(char* fileName) {
    FILE * filePtr;
    filePtr = fopen(fileName, "r");
    if (filePtr == NULL) {
        printf("Unable to load file '%s'\n", fileName);
        return 0;
    }

    int matrixCount = 0;
    char readBuffer[BUFFER_SIZE];
    while ( fgets(readBuffer, BUFFER_SIZE, filePtr) != NULL ) {
        if ( charArrayIsBlank(readBuffer, BUFFER_SIZE) > 0 ) {
            matrixCount++;
        }
    }
    matrixCount++;

    fclose(filePtr);

    return matrixCount;
}

/*
* Loads all matrices in a file in the specified format and returns array of struct Matrix's
*/
struct Matrix* loadFromFile (char * fileName) {
    /// Check file name isn't blank
    if (!fileName) {
        printf("Unable to find file '%s'\n", fileName);
        return NULL;
    }

    /// Get initial matrix size first
    int totalMatrixCount = getFileMatrixCount(fileName);
    printf("Total '%d' matrices\n", totalMatrixCount);

    /// Open reference to the file
    FILE * filePtr;
    filePtr = fopen(fileName, "r");
    if (filePtr == NULL) {
        printf("Unable to load file '%s'\n", fileName);
        return NULL;
    }

    /// All matrices in the file
    struct Matrix* matrices = malloc( sizeof(struct Matrix) * totalMatrixCount );

    struct MatrixSize* currentMatrixSize = malloc(sizeof(struct MatrixSize*));
    currentMatrixSize->x = currentMatrixSize-> y = 0;

    double* currentMatrixValues = NULL;
    int currentMatrixCount = 0;
    int currentMatrixCurrentLineCount = 0;

    char readBuffer[BUFFER_SIZE];
    while ( fgets(readBuffer, BUFFER_SIZE, filePtr) != NULL ) {
        struct MatrixSize* mtxSize = charArrayIsMatrixSize(readBuffer, BUFFER_SIZE);
        if ( mtxSize != NULL && currentMatrixSize->x == 0 && currentMatrixSize->y == 0 ) {
            /// Parse X by Y matrix value line
            currentMatrixSize = mtxSize;

            printf("Matrix %d %dx%d\n", currentMatrixCount, currentMatrixSize->x, currentMatrixSize->y);
        } else if ( charArrayIsBlank(readBuffer, BUFFER_SIZE) > 0 ) {
            /// blank line, end of parsing this matrix
            printf("Finished parsing matrix\n");
            
            struct Matrix thisMatrix;
            thisMatrix.values = currentMatrixValues;
            thisMatrix.size = *currentMatrixSize;

            matrices[currentMatrixCount] = thisMatrix;

            // Increment matrix count & parse new matrix
            currentMatrixCount++;

            // Reset vars that store data for next matrix
            currentMatrixCurrentLineCount = 0;
            currentMatrixSize->x = currentMatrixSize->y = 0;
            // free previous matrix vals before disposal
            free(currentMatrixValues);
            currentMatrixValues = NULL;
        } else {
            /// On a new matrix, malloc new set of values, length of Y
            if (currentMatrixValues == NULL) {
                currentMatrixValues = malloc( sizeof(double) * currentMatrixSize->y);
            }
            /// Parse current line
            double* values = parseLineOfDoubles(readBuffer, *currentMatrixSize);
            /// Insert parsed values into current matrix values array
            for (int i = 0; i < currentMatrixSize->y; i++) {
                int index = (currentMatrixCurrentLineCount * currentMatrixSize->y) + i;
                currentMatrixValues[index] = values[i];

                printf("values[%d][%d] = %lf\n", currentMatrixCurrentLineCount, i, values[i]);
            }
            currentMatrixCurrentLineCount++;
        }
    }
    /// Reached end of file
    //printf("Reached EOF\n");

    /// Final increment of matrixCount and insert final values
    struct Matrix thisMatrix;
    thisMatrix.values = currentMatrixValues;
    thisMatrix.size = *currentMatrixSize;

    matrices[currentMatrixCount] = thisMatrix;

    // Increment matrix count & parse new matrix
    currentMatrixCount++;

    /// Close and return matrix
    fclose(filePtr);

    /// Return the parsed matrices
    return matrices;
}

int main()
{
    char* filePath = "/Users/joshshepherd/Documents/Development/6CS005-assignment/matrix-multiply/matrices-final.txt";

    struct Matrix* Matrices;
    int totalMatrixCount = getFileMatrixCount(filePath);
    Matrices = loadFromFile(filePath);

    for (int i = 0; i < totalMatrixCount; i++) {
        printf("Matrix[%d] has dimension %dx%d\n", i, Matrices[i].size.x, Matrices[i].size.y);
    }

    printf("Total of '%d' matrices!", totalMatrixCount);
}

