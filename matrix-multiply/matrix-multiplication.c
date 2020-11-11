/// Josh Shepherd 1700471
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "mtx-file-handler.c"

/// All loaded matrices in the parsed file
struct Matrix** Matrices;

struct ThreadArgs {
    /// Current thread index number
    int threadIndex;
    /// Amount to iterate over
    int dotProductCount;
    /// Final matrix info
    struct Matrix* resultMatrixInfo;
    /// All Matrix A values
    double* matrixAValues;
    /// All Matrix B values
    double* matrixBValues;
};

/// Define functions
void printMatrix(double* matrix, int xCount, int yCount);
void* calculate(void* param);
int canMultiplyMatrices(struct Matrix a, struct Matrix b);

/*
*  Takes a single file path argument pointing to a file in the specified format
*/
int main (int argc, char* argv[]) {

    char* filePath = "/Users/joshshepherd/Documents/Development/6CS005-assignment/matrix-multiply/matrices-final.txt";
    char* outputFilePath = "/Users/joshshepherd/Documents/Development/6CS005-assignment/matrix-multiply/matrices-output.txt";

    /// Parses matrices file and loads into memory
    int totalMatrixCount = getFileMatrixCount(filePath);
    Matrices = loadFromFile(filePath);

    printf("Successfully loaded '%d' matrices from file '%s'\n", totalMatrixCount, filePath);
    
    /// Print out loaded matrices and their values
    // for(int i = 0; i < totalMatrixCount; i++) {
    //     printf("Matrix '%d' = %dx%d\n", i, Matrices[i]->size.x, Matrices[i]->size.y);
    //     printMatrix(Matrices[i]->values, Matrices[i]->size.x, Matrices[i]->size.y);
    // }

    /// Loop over matrices and perform multiplication
    for (int i = 0; i < totalMatrixCount; i += 2) {
        struct Matrix matrixA = *(Matrices[i]);
        struct Matrix matrixB = *(Matrices[i + 1]);

        printf("Matrix A:\n");
        printMatrix(matrixA.values, matrixA.size.x, matrixA.size.y);

        printf("Matrix B:\n");
        printMatrix(matrixB.values, matrixB.size.x, matrixB.size.y);

        struct Matrix* resultMatrix = malloc( sizeof(struct Matrix) );
        resultMatrix->size.x = matrixA.size.x;
        resultMatrix->size.y = matrixB.size.y;

        // Set memory for final matrix values
        resultMatrix->values = malloc( sizeof(double) * resultMatrix->size.x * resultMatrix->size.y );

        // Check if matrixA and matrixB can be multiplied together
        int canMultiply = canMultiplyMatrices(matrixA, matrixB);
        if (canMultiply == 1) 
        {
            /// Use one thread per value to calculate
            int threadCount = resultMatrix->size.x * resultMatrix->size.y;
            /// Create and store array of threads used for pthreads
            pthread_t* threadIds = malloc( sizeof(pthread_t) * threadCount);
            
            for (int i = 0; i < threadCount; i++) {
                struct ThreadArgs *args = malloc(sizeof(*args));
                if (args != NULL) 
                {
                    /// Send final matrix array index as threadIndex
                    args->threadIndex = i;
                    /// Amount of values needed to make the dot product
                    /// which is equal to Matrix A columns or Matrix B rows
                    args->dotProductCount = resultMatrix->size.y;

                    // Set result matrix ptr & matrix a and b pointers
                    args->resultMatrixInfo = resultMatrix;
                    args->matrixAValues = matrixA.values;
                    args->matrixBValues = matrixB.values;

                    pthread_create( &threadIds[i], NULL, calculate, args);
                }
            }

            printf("Created %d threads to create a %d by %d matrix\n", threadCount, resultMatrix->size.x, resultMatrix->size.y);

            /// Await the threads to finish calculations
            for (int i = 0; i < threadCount; i++) {
                pthread_join( threadIds[i], NULL );
            }
            
            printf("Result of Matrix[%d] * Matrix[%d] =\n", i, i + 1);
            printf("Result Matrix:\n");
            printMatrix(resultMatrix->values, resultMatrix->size.x, resultMatrix->size.y);
        }
        else 
        {
            // Unable to multiply, dump info to console
            printf("Unable to multiply Matrices[%d] '%dx%d' and  Matrices[%d] '%dx%d'\n", i, matrixA.size.x, matrixA.size.y, i + 1, matrixB.size.x, matrixB.size.y);
        }

        // Clean up malloc
        free(resultMatrix->values);
    }
    return 0;
}

/*
* Muiltiplies matrices passed by ThreadArgs struct
*/
void* calculate (void* param) {
    /// Convert the void pointer to our struct
    struct ThreadArgs* args = (struct ThreadArgs*)param;
    
    struct MatrixSize resultMatrixSize = args->resultMatrixInfo->size;
    int index = args->threadIndex;

    /// Determine current column and row index for Matrix A and B to create Dot product
    int col = index %  resultMatrixSize.x;
    int row = (index - col) / resultMatrixSize.y;
    
    /// Check we are within bounds of final matrix
    if ( row >= resultMatrixSize.x || col >= resultMatrixSize.y ) {
        //printf("Too many threads\n");
        return 0;
    }

    /// Work out total of multiplication by accessing row of Matrix A and column of Matrix B
    double total = 0;
    for (int i = 0; i < args->dotProductCount; i++) {
        /// Find A and B positions
        int mtxAThisIndex = (row * (resultMatrixSize.x - 1)) + i;
        int mtxBThisIndex = (i * resultMatrixSize.y) + col;

        // Finally multiply and add to cumulative total
        double aVal = args->matrixAValues[ mtxAThisIndex ];
        double bVal = args->matrixBValues[ mtxBThisIndex ];
        total += aVal * bVal;
    }

    /// Insert value into final matrix
    args->resultMatrixInfo->values[args->threadIndex] = total;
    //*(MATRIX_Z + args->threadIndex) = total;

    return NULL;
}

/* 
* Prints out the matrix array to the console with it's indexes and it's values 
*/
void printMatrix (double* matrix, int xCount, int yCount) {
    for (int i = 0; i < xCount; i++) {
        for(int j = 0; j < yCount; j++) {
            printf("%f", *(matrix + (i * yCount) + j));

            /// Print white space if not reached final column
            if ( j < yCount - 1 ) {
                printf("    ");
            }
        }
        printf("\n");
    }
}

/*
* Returns true if matrix a can be multiplied against matrix b
*/
int canMultiplyMatrices(struct Matrix a, struct Matrix b) {
    // Only able to multiply if Matrix A Y column count
    // is equal to MatrixB X row count
    if (a.size.y == b.size.x) {
        return 1;
    }
    return 0;
}