# 6CS005 - C & CUDA

## Matrix Multiplication

Written in C and uses pthreads. Program is able to parse a file in a certain format [such as 1700471-matrices.txt](./matrix-multiply/1700471-matrices.txt)) that contains matrices with their sizes and then their values. Matrices are separated with a new line character. Program is able to parse this file, take the first two matrices and multiply if it is possible. Once complete, moves on to the next pair of matrices. Results get stored and printed in a final file.

### Compile Command:
````
cc matrix-multiplication.c -pthread 
````
### Run Command:
````
./a.out "1700471-matrices.txt" "output-matrices.txt"
````
## Password Cracking

Written in C, using pthreads & crypt. Able to crack a SHA512 encrypted string from a password in the format of 'AA00' (a capital letter, a capital letter and a number between 0-99)

### Compile Command:
````
cc pass-crack.c -lcrypt -pthread
````
### Run Command:
````
./a.out "$6$AS$wKDMKDtx/s3ILNkNaRNFIM0w81/weD1UZ8daNhbQBXuj8L.7OY4trHnSraeizmFYrMwjlb1uRTPxu20rqhmMn/"
````

## CUDA: Password Cracking

Written for CUDA, able to crack a 4 digit password in the format of 'AA00' (two capital letters and a number between 0-99) that's encrypted through the [CUDACrypt](https://github.com/JoshLmao/6CS005-assignment/blob/main/pass-crack-cuda/pass-crack-cuda.cu#L7) function. 

### Compile Command:
````
nvcc pass-crack-cuda.cu
````
### Run Command:
````
./a.out "ccbddb7362"
````

## CUDA: Gaussian Blur

Written for CUDA and uses [lodepng](https://lodev.org/lodepng/) to parse a png file and applies a gaussian blur filter to the image.

### Compile Command:
````
nvcc gaussian-blur.cu lodepng.cpp
````
### Run Command:
````
./a.out "img.png" "output-blurred.png"
````