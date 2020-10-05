#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// Set matrix X and Y lengths as constants
#define A_X 3
#define A_Y 3
// Inverse A_X and A_Y for second matrix
#define B_X 3
#define B_Y 3
// Final matrix dimensions are Y size of matrix A by X size of matrix B
int Z_X = A_X;
int Z_Y = B_Y;

/// Pointer to MATRIX_A with row length of A_X, col length of A_Y
int* MATRIX_A;
/// Pointer to MATRIX_B with row length of B_X, col length of B_Y
int* MATRIX_B;
/// Pointer to final matrix
int* MATRIX_Z;

void printMatrix(int* matrix, int xCount, int yCount);
void populateMatrix(int* matrix, int xCount, int yCount);
void* calculate(void* param);

int main (int argc, char* argv[]) {
    /// Init and populate MATRIX_A with rand values
    MATRIX_A = (int*)malloc( sizeof(int) * A_X * A_Y);
    populateMatrix(MATRIX_A, A_X, A_Y);
    
    printf("Matrix A:\n");
    printMatrix(MATRIX_A, A_X, A_Y);

    /// Init and populate MATRIX_B with rand values
    MATRIX_B = (int*)malloc( sizeof(int) * B_X * B_Y);
    populateMatrix(MATRIX_B, B_X, B_Y);
    
    printf("Matrix B:\n");
    printMatrix(MATRIX_B, B_X, B_Y);

    /// Init final matrix for inserting calculations
    MATRIX_Z = (int*)malloc( sizeof(int) * Z_X * Z_Y);
    //printMatrix(MATRIX_Z, Z_X, Z_Y);
    
    /// Use one thread per value to calculate
    int threadCount = Z_X * Z_Y;
    /// Create and store array of threads used for pthreads
    pthread_t* threadIds = malloc( sizeof(pthread_t) * threadCount);
    
    for (int i = 0; i < threadCount; i++) {
        int *arg = malloc(sizeof(*arg));
        if (arg != NULL) 
        {
            *arg = i;
            pthread_create( &threadIds[i], NULL, calculate, arg);
        }
    }

    printf("Created %d threads to create a %d by %d matrix\n", threadCount, Z_X, Z_Y);

    /// Await the threads to finish calculations
    for (int i = 0; i < threadCount; i++) {
        pthread_join( threadIds[i], NULL);
    }
    
    printf("Result of Matrix A * Matrix B =\n");
    printf("Matrix Z:\n");
    printMatrix(MATRIX_Z, Z_X, Z_Y);
    
    return 0;
}

/*
* Muiltiplies the MATRIX_A and MATRIX_B position
*/
void* calculate (void* param) {
    // Get final matrix cell index from param
    int* cellIndex = (int*)param;
    
    // Figure out column from modulus of final matrix Y size
    int col = *cellIndex % Z_Y;
    // Figure out which row by dividing by final matrix X size
    int row = *cellIndex / Z_X;
    
    //printf("Threads id %ld using row %d and col %d from index %d\n", pthread_self(), row, col, *cellIndex);
    
    // Work out total of multiplication by accessing row of Matrix A and
    // column of Matrix B
    int total = 0;
    for (int i = 0; i < A_Y; i++) {
        int aVal = *(MATRIX_A + i + (A_X * row));
        int bVal = *(MATRIX_B + (i * B_Y) + col);
        total += (aVal * bVal);
    }
    
    //printf("Value at row %d col %d is is %d\n", row, col, total);

    /// Insert value into final matrix
    *(MATRIX_Z + *cellIndex) = total;
}

/* 
* Prints out athe matrix array to the console with it's indexes and it's values 
*/
void printMatrix (int* matrix, int xCount, int yCount) {
    for (int i = 0; i < xCount; i++) {
        for(int j = 0; j < yCount; j++) {
            printf("%d    ", *(matrix + (i * yCount) + j));
        }
        printf("\n");
    }
}

/* 
* Populates a matrix with random numbers inbetween 20
*/
void populateMatrix (int* matrix, int xCount, int yCount) {
    for (int i = 0; i < xCount * yCount; i++) {
        *(matrix + i) = rand() % 20; //i;
    }
}