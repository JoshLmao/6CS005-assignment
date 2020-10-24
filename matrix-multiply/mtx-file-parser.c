#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

struct MatrixSize {
    int x;
    int y;
};

struct Matrix {
    struct MatrixSize size;
    double* values;
};

int charArrayIsMatrixSize (char * chars, int size) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        // Check for x char
        if (chars[i] == 'x') {
            return 1;
        }
        // Check if reached end of line
        if (chars[i] == '\n') {
            return 0;
        }
    }
    return 0;
}

int charArrayIsBlank (char * chars, int size) {
    return chars[0] == '\n';
}

double* parseLineOfDoubles(char* lineChars, struct MatrixSize mtxSize) {
    double* lineVals = malloc (sizeof(double) * mtxSize.y);
    int lineYCount = 0;

    char* split = strtok(lineChars, ",");
    while (split != NULL) {
        double parsed = atof(split);
        if ( parsed > 0 ) {
            lineVals[lineYCount] = parsed;
            lineYCount += 1;
        }

        // Move split onto next split index
        split = strtok(NULL, ",");
    }

    return lineVals;
}

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

    struct MatrixSize* currentMatrixSize;
    double* currentMatrixValues = NULL;
    int currentMatrixCount = 0;
    int currentMatrixCurrentLineCount = 0;

    char readBuffer[BUFFER_SIZE];
    while ( fgets(readBuffer, BUFFER_SIZE, filePtr) != NULL ) {
        if (charArrayIsMatrixSize(readBuffer, BUFFER_SIZE) > 0) {
            /// Parse X by Y matrix value line
            currentMatrixSize = malloc( sizeof(struct MatrixSize) );
            currentMatrixSize->x = atoi(&readBuffer[0]);
            currentMatrixSize->y = atoi(&readBuffer[2]);

            printf("Matrix %d %dx%d\n", currentMatrixCount, currentMatrixSize->x, currentMatrixSize->y);
        } else if ( charArrayIsBlank(readBuffer, BUFFER_SIZE) > 0 ) {
            /// blank line, end of parsing this matrix
            printf("FINISHED PARSING MATRIX\n");
            
            struct Matrix thisMatrix;
            thisMatrix.values = currentMatrixValues;
            thisMatrix.size = *currentMatrixSize;

            matrices[currentMatrixCount] = thisMatrix;

            // Increment matrix count & parse new matrix
            currentMatrixCount++;

            currentMatrixCurrentLineCount = 0;
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

            // for(int i = 0; i < currentMatrixSize->y; i++) {
            //     printf("values[%d] = %lf\n", i, values[i]);
            // }
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
    char* filePath = "/Users/joshshepherd/Documents/Development/6CS005-assignment/matrix-multiply/test-mtx-file.txt";

    struct Matrix* Matrices;
    int totalMatrixCount = getFileMatrixCount(filePath);
    Matrices = loadFromFile(filePath);

    for (int i = 0; i < totalMatrixCount; i++) {
        printf("Matrix[%d] has dimension %dx%d\n", i, Matrices[i].size.x, Matrices[i].size.y);
    }

    printf("Total of '%d' matrices!", totalMatrixCount);
}

