#include <stdio.h>
#include <stdlib.h>
#include "lodepng.h"

int main(int argc, char** argv) {
	char* loadFilePath = "./img.png";
	
	unsigned int width, height;
	unsigned char* values;
	lodepng_decode32_file(&values, &width, &height, loadFilePath);
	
	if (width <= 0 || height <= 0) {
		printf("Error occured loading PNG\n");
		exit(1);
	}
	
	printf("Success\n");
}
