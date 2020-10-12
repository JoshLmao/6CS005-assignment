/// Josh Shepherd 1700471
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

struct MatrixSize {
    /// X dimension of a mtrix
    int x;
    /// X dimension of a matrix
    int y;
};

/// size of Matrix A
struct MatrixSize MATRIX_A_SIZE;
/// size of Matrix B
struct MatrixSize MATRIX_B_SIZE;
// Final matrix dimensions are Y size of matrix A by X size of matrix B
int Z_X;
int Z_Y;

/// Pointer to MATRIX_A with row length of A_X, col length of A_Y
int* MATRIX_A;
/// Pointer to MATRIX_B with row length of B_X, col length of B_Y
int* MATRIX_B;
/// Pointer to final matrix
int* MATRIX_Z;

struct MatrixSize getMatrixFileSize (char * fileName);
int * loadMatrixFromFile(char * fileName, struct MatrixSize size);
void printMatrix(int* matrix, int xCount, int yCount);
void populateMatrix(int* matrix, int xCount, int yCount);
void* calculate(void* param);
void cleanup();

/*
*   Accepts two file paths as command line args. Must be full paths to the files that
*   contain matrices. Files should be numbers separated by spaces and new lines to
*   define it's X and Y
*/
int main (int argc, char* argv[]) {

    /// Load matrices from file paths from arguments
    char * matrixAFile = "./matrixA.txt";
    if (argc >= 1) {
        matrixAFile = argv[1];
    }
    char * matrixBFile = "./matrixB.txt";
    if (argc >= 2) {
        matrixBFile = argv[2];
    }

    /// Determine matrix sizes of files
    struct MatrixSize matrixASize, matrixBSize;
    matrixASize = getMatrixFileSize(matrixAFile);
    matrixBSize = getMatrixFileSize(matrixBFile);

    /// Load specified files and check they are valid
    MATRIX_A = loadMatrixFromFile(matrixAFile, matrixASize);
    MATRIX_B = loadMatrixFromFile(matrixBFile, matrixBSize);

    if ( MATRIX_A == NULL || MATRIX_B == NULL ) {
        printf("Unable to load one or both of the specified files.\n");
        return 0;
    }

    /// Print out loaded A and B matrices
    printf("Matrix A:\n");
    printMatrix(MATRIX_A, matrixASize.x, matrixASize.y);
    printf("Matrix B:\n");
    printMatrix(MATRIX_B, matrixBSize.x, matrixBSize.y);

    /// Check if matrices can be multiplied together
    /// Matrix A Y and Matrix B X must match
    if ( matrixASize.y != matrixBSize.x ) {
        printf("Cannot multiply matrices, sorry ;( \n");
        cleanup();
        return 0;
    }

    /// matrices can be multiplied, go ahead

    /// Set shape of final matrix (outside matrix numbers)
    Z_X = matrixASize.x;
    Z_Y = matrixBSize.y;

    /// Initialize final matrix for inserting calculations
    MATRIX_Z = (int*)malloc( sizeof(int) * Z_X * Z_Y);
    
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
    
    cleanup();
    return 0;
}

/*
* Loads a matrix from a file and returns a 2x2 array of the matrix.
* Can return null if no file is found or able to be loaded
*/
int * loadMatrixFromFile (char * fileName, struct MatrixSize size) {
    /// Check file name isn't blank
    if (!fileName) {
        printf("Unable to find file '%s'\n", fileName);
        return NULL;
    }

    /// Open reference to the file
    FILE * filePtr;
    filePtr = fopen(fileName, "r");
    if (filePtr == NULL) {
        printf("Unable to load file '%s'\n", fileName);
        return NULL;
    }

    /// Malloc appropriate matrix once size is known
    int * matrix = malloc ( sizeof(int) * size.x * size.y );

    int currentMatrixIndex = 0;
    char c;
    for ( c = getc(filePtr); c != EOF; c = getc(filePtr) ) {
        int currentVal = 0;

        /// Determine if char is a number
        if ( c != ' ' && c != '\n') {
            currentVal = atoi(&c);
            matrix[currentMatrixIndex] = currentVal;

            /// Increment index once found
            currentMatrixIndex++;
        }
    }

    /// Close and return matrix
    fclose(filePtr);
    return matrix;
}

/*
* Gets the Matrix size of the file. Can return blank struct if no file found
*/
struct MatrixSize getMatrixFileSize (char * fileName) {
    struct MatrixSize size;
    size.x = 0;
    size.y = 0;
    
    /// Open a pointer and check its valid
    FILE * filePtr = fopen(fileName, "r");
    if (filePtr == NULL) {
        return size;
    }
    
    /// Iterate through file counting X and Y either by white space or new line
    char c;
    /// Start Y at 1 since file exists and starting on first line
    size.y = 1;
    for ( c = getc(filePtr); c != EOF; c = getc(filePtr) ) {
        /// Increment Y size on new line
        if ( c == '\n' ) {
            size.y++;
        }
        /// Only count X size during first line
        if (size.y == 1 && c != ' ') {
            size.x++;
        }
    }

    // Close file pointer and return size
    fclose(filePtr);
    return size;
}

/*
* Muiltiplies the MATRIX_A and MATRIX_B position
*/
void* calculate (void* param) {
    // Get final matrix cell index from param
    int* cellIndex = (int*)param;
    
    // Figure out column from modulus of final matrix Y size
    int col = *cellIndex % Z_X;
    // Figure out which row by dividing by final matrix X size
    int row = (*cellIndex - row) / Z_Y;
    
    //printf("Threads id %ld using row %d and col %d from index %d\n", pthread_self(), row, col, *cellIndex);
    
    if (row >= Z_X || row >= Z_Y) {
        printf("Too many threads");
        return 0;
    }

    // Work out total of multiplication by accessing row of Matrix A and
    // column of Matrix B
    int total = 0;
    for (int i = 0; i < Z_Y; i++) {
        int aVal = MATRIX_A[ row + i * Z_X ];
        int bVal = MATRIX_B[ i + col * Z_Y ];
        total += aVal * bVal;
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

/*
* Cleans up any malloc'd variables in program
*/
void cleanup() {
    free(MATRIX_A);
    free(MATRIX_B);
    free(MATRIX_Z);
}