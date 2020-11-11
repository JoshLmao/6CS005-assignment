/// Josh Shepherd - 1700471
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// Size of buffer when parsing a line from the file
#define BUFFER_SIZE 256

/*
* X and Y size of the matrix
*/
struct MatrixSize {
    /// X dimension of a matrix
    int x;
    /// X dimension of a matrix
    int y;
};

/*
* One Matrix that contains it's size and values in a 2D array
*/
struct Matrix {
    /// X and Y dimensions of the array
    struct MatrixSize size;
    /// 2D array containing all matrix values
    double* values;
};

/*
* Checks if the char array line is the matrix definition line and returns the size is true
* (AxB where A and B is the matrix size)
*/
struct MatrixSize* charArrayIsMatrixSize (char * chars, int size) {
    struct MatrixSize* mtxSize = (struct MatrixSize*) malloc( sizeof(struct MatrixSize) );
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
* Gets the total matrices in the specified file format and returns the count
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
struct Matrix** loadFromFile (char * fileName) {
    /// Check file name isn't blank
    if (!fileName) {
        printf("Unable to find file '%s'\n", fileName);
        return NULL;
    }

    /// Get initial matrix size first
    int totalMatrixCount = getFileMatrixCount(fileName);
    //printf("Total '%d' matrices\n", totalMatrixCount);

    /// Open reference to the file
    FILE * filePtr;
    filePtr = fopen(fileName, "r");
    if (filePtr == NULL) {
        printf("Unable to load file '%s'\n", fileName);
        return NULL;
    }

    /// All matrices in the file
    struct Matrix** matrices = malloc( sizeof(struct Matrix) * totalMatrixCount );

    struct MatrixSize* currentMatrixSize = (struct MatrixSize*) malloc(sizeof(struct MatrixSize));
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

            //printf("Matrix %d %dx%d\n", currentMatrixCount, currentMatrixSize->x, currentMatrixSize->y);
        } else if ( charArrayIsBlank(readBuffer, BUFFER_SIZE) > 0 ) {
            /// blank line, end of parsing this matrix
            //printf("Finished parsing matrix\n");
            
            struct Matrix* thisMatrix = malloc( sizeof(struct Matrix) );
            thisMatrix->values = currentMatrixValues;
            thisMatrix->size = *currentMatrixSize;

            matrices[currentMatrixCount] = thisMatrix;

            // Increment matrix count & parse new matrix
            currentMatrixCount++;

            // Reset vars that store data for next matrix
            currentMatrixCurrentLineCount = 0;
            currentMatrixSize->x = currentMatrixSize->y = 0;
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

                //printf("values[%d][%d] = %lf\n", currentMatrixCurrentLineCount, i, values[i]);
            }
            currentMatrixCurrentLineCount++;
        }
    }
    /// Reached end of file
    //printf("Reached EOF\n");

    /// Final increment of matrixCount and insert final values
    struct Matrix* thisMatrix = malloc( sizeof(struct Matrix) );
    thisMatrix->values = currentMatrixValues;
    thisMatrix->size = *currentMatrixSize;

    matrices[currentMatrixCount] = thisMatrix;

    // Increment matrix count & parse new matrix
    currentMatrixCount++;

    /// Close and return matrix
    fclose(filePtr);

    /// Return the parsed matrices
    return matrices;
}

/*
* Saves an array of matrices to the specified filePath in the specified format
* Format: Print AxB, then each matrix value comma separated, then new line char to separate each matrix
*/
int saveMatricesToFile ( char* filePath, struct Matrix** matrices, int matricesCount) {
    /// Open save to file path and check if it's successful
    FILE* saveToFile = fopen(filePath, "w");
    if (saveToFile == NULL)
    {
        printf("Unable to open filePath '%s'. Check it isn't open\n", filePath);
        return 0;
    }

    /// Loop over every matrix to print inside file
    for( int i = 0; i < matricesCount; i++ ) {
        struct Matrix matrix = *(matrices[i]);

        /// Write matrix size first
        fprintf(saveToFile, "%d,%d\n", matrix.size.x, matrix.size.y);

        /// Input all matrix file values separated with commas
        for (int j = 0; j < matrix.size.x; j++) {
            int rowIndex = (j * matrix.size.y);
            for( int k = 0; k < matrix.size.y; k++ ) {
                /// Input matrix value into file
                fprintf(saveToFile, "%f", matrix.values[rowIndex + k]);
                /// Print a comma until second but last value
                if (k < matrix.size.y - 1) {
                    fprintf(saveToFile, ",");
                }
            }
            /// Print new line and repeat next line of values
            fprintf(saveToFile, "\n");
        }
        /// Input separator of matrices and start again
        fprintf(saveToFile, "\n");
    }

    /// Close the file and return success
    fclose(saveToFile);
    return 1;
}

/* 
* Testing Main for parsing one file
*/
/*
int main()
{
    char* filePath = "/Users/joshshepherd/Documents/Development/6CS005-assignment/matrix-multiply/matrices-final.txt";
    char* saveToPath = "/Users/joshshepherd/Documents/Development/6CS005-assignment/matrix-multiply/matrices-output.txt";

    struct Matrix** Matrices;
    int totalMatrixCount = getFileMatrixCount(filePath);
    Matrices = loadFromFile(filePath);

    for (int i = 0; i < totalMatrixCount; i++) {
        printf("Matrix[%d] has dimension %dx%d\n", i, Matrices[i]->size.x, Matrices[i]->size.y);
    }

    printf("Total of '%d' matrices!", totalMatrixCount);

    int success = saveMatricesToFile(saveToPath, Matrices, totalMatrixCount);
    if (success == 1)
        printf("Successfully saved to file '%s'\n", saveToPath);
    else
        printf("Unable to save to file '%s'\n", saveToPath);
}
*/