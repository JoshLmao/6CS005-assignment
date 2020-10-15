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
struct MatrixSize MATRIX_Z_SIZE;

/// Pointer to MATRIX_A with row length of A_X, col length of A_Y
int* MATRIX_A;
/// Pointer to MATRIX_B with row length of B_X, col length of B_Y
int* MATRIX_B;
/// Pointer to final matrix
int* MATRIX_Z;

struct ThreadArgs {
    /// Number
    int threadIndex;
    int dotProductCount;
};

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
    MATRIX_A_SIZE = getMatrixFileSize(matrixAFile);
    MATRIX_B_SIZE = getMatrixFileSize(matrixBFile);

    /// Load specified files and check they are valid
    MATRIX_A = loadMatrixFromFile(matrixAFile, MATRIX_A_SIZE);
    MATRIX_B = loadMatrixFromFile(matrixBFile, MATRIX_B_SIZE);

    if ( MATRIX_A == NULL || MATRIX_B == NULL ) {
        printf("Unable to load one or both of the specified files.\n");
        return 0;
    }

    /// Print out loaded A and B matrices
    printf("Matrix A:\n");
    printMatrix(MATRIX_A, MATRIX_A_SIZE.x, MATRIX_A_SIZE.y);
    printf("Matrix B:\n");
    printMatrix(MATRIX_B, MATRIX_B_SIZE.x, MATRIX_B_SIZE.y);

    /// Check if matrices can be multiplied together
    /// Matrix A Y and Matrix B X must match
    if ( MATRIX_A_SIZE.y == MATRIX_B_SIZE.x ) {
       printf("Able to multiply a %dx%d and %dx%d matrices\n", MATRIX_A_SIZE.x, MATRIX_A_SIZE.y, MATRIX_B_SIZE.x, MATRIX_B_SIZE.y);
    } else {
        printf("Cannot multiply matrices, sorry ;( \n");
        cleanup();
        return 0;
    }

    /// matrices can be multiplied, go ahead

    /// Set shape of final matrix (outside matrix numbers)
    MATRIX_Z_SIZE.x = MATRIX_A_SIZE.x;
    MATRIX_Z_SIZE.y = MATRIX_B_SIZE.y;

    /// Initialize final matrix for inserting calculations
    MATRIX_Z = (int*)malloc( sizeof(int) * MATRIX_Z_SIZE.x * MATRIX_Z_SIZE.y);
    
    /// Use one thread per value to calculate
    int threadCount = MATRIX_Z_SIZE.x * MATRIX_Z_SIZE.y;
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
            args->dotProductCount = MATRIX_A_SIZE.y;
            pthread_create( &threadIds[i], NULL, calculate, args);
        }
    }

    printf("Created %d threads to create a %d by %d matrix\n", threadCount, MATRIX_Z_SIZE.x, MATRIX_Z_SIZE.y);

    /// Await the threads to finish calculations
    for (int i = 0; i < threadCount; i++) {
        pthread_join( threadIds[i], NULL );
    }
    
    printf("Result of Matrix A * Matrix B =\n");
    printf("Matrix Z:\n");
    printMatrix(MATRIX_Z, MATRIX_Z_SIZE.x, MATRIX_Z_SIZE.y);
    
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
    size.x = 1;
    for ( c = getc(filePtr); c != EOF; c = getc(filePtr) ) {
        /// Increment Y size on new line
        if ( c == '\n' ) {
            size.x++;
        }
        /// Only count X size during first line
        if (size.x == 1 && c != ' ') {
            size.y++;
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
    /// Convert the void pointer to our struct
    struct ThreadArgs* args = (struct ThreadArgs*)param;
    
    int index = args->threadIndex;

    /// Determine current column and row index for Matrix A and B to create Dot product
    int col = index % MATRIX_Z_SIZE.x;
    int row = (index - col) / MATRIX_Z_SIZE.y;
    
    /// Check we are within bounds of final matrix
    if ( row >= MATRIX_Z_SIZE.x || col >= MATRIX_Z_SIZE.y ) {
        //printf("Too many threads\n");
        return 0;
    }

    /// Work out total of multiplication by accessing row of Matrix A and column of Matrix B
    int total = 0;
    for (int i = 0; i < args->dotProductCount; i++) {
        /// Find A and B positions
        int mtxAThisIndex = (row * (MATRIX_A_SIZE.x - 1)) + i;
        int mtxBThisIndex = (i * MATRIX_B_SIZE.y) + col;

        // Finally multiply and add to cumulative total
        int aVal = MATRIX_A[ mtxAThisIndex ];
        int bVal = MATRIX_B[ mtxBThisIndex ];
        total += aVal * bVal;
    }

    /// Insert value into final matrix
    *(MATRIX_Z + args->threadIndex) = total;

    return NULL;
}

/* 
* Prints out the matrix array to the console with it's indexes and it's values 
*/
void printMatrix (int* matrix, int xCount, int yCount) {
    for (int i = 0; i < xCount; i++) {
        for(int j = 0; j < yCount; j++) {
            printf("%d", *(matrix + (i * yCount) + j));

            /// Print white space if not reached final column
            if ( j < yCount - 1 ) {
                printf("    ");
            }
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