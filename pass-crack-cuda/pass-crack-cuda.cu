// Josh Shepherd - 1700471
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__device__
char* CudaCrypt(char* rawPassword) {

	char * newPassword = (char *) malloc(sizeof(char) * 11);

	newPassword[0] = rawPassword[0] + 2;
	newPassword[1] = rawPassword[0] - 2;
	newPassword[2] = rawPassword[0] + 1;
	newPassword[3] = rawPassword[1] + 3;
	newPassword[4] = rawPassword[1] - 3;
	newPassword[5] = rawPassword[1] - 1;
	newPassword[6] = rawPassword[2] + 2;
	newPassword[7] = rawPassword[2] - 2;
	newPassword[8] = rawPassword[3] + 4;
	newPassword[9] = rawPassword[3] - 4;
	newPassword[10] = '\0';

	for(int i =0; i<10; i++){
		if(i >= 0 && i < 6){ //checking all lower case letter limits
			if(newPassword[i] > 122){
				newPassword[i] = (newPassword[i] - 122) + 97;
			}else if(newPassword[i] < 97){
				newPassword[i] = (97 - newPassword[i]) + 97;
			}
		}else{ //checking number section
			if(newPassword[i] > 57){
				newPassword[i] = (newPassword[i] - 57) + 48;
			}else if(newPassword[i] < 48){
				newPassword[i] = (48 - newPassword[i]) + 48;
			}
		}
	}
	return newPassword;
}

/**
	Checks if one char string matches another char string of the length
*/
__device__ 
int isEncryptedMatching(char* one, char* two, int length) {
	int result = 1;
	for (int i = 0; i < length; i++) {
		if (one[i] != two[i]) {
			result = 0;
			break;
		}
	}
	return result;
}

/**
	Decrypts a pass using a CUDA thread
*/
__global__
void decryptPass(char* alphabet, char* numbers, char* encryptedPass, char* deviceOutputPass)
{
	/// Get the unique cuda thread id
	int uid = blockDim.x * blockIdx.x + threadIdx.x;
	
	/// Check if another thread found output pass before starting
	if(*deviceOutputPass != NULL) {
		//printf("OutputPass not null! Cancelling CUDA thread '%d'\n", uid);
		return;
	}

	/// Create potential pass to check on this thread
	char potentialPass[4];
	potentialPass[0] = alphabet[blockIdx.x];
	potentialPass[1] = alphabet[blockIdx.y];
	potentialPass[2] = numbers[threadIdx.x];
	potentialPass[3] = numbers[threadIdx.y];
	
	/// Encrypt the potential password
	char* encryptedPotential;
	encryptedPotential = CudaCrypt(potentialPass);
	
	//printf("UID: '%d' Plain: '%s' Encrypted Plain: '%s' Target Encrypted: '%s'\n", uid, potentialPass, encryptedPotential, encryptedPass);
	
	/// Check the current potential pass is matches the target encryptedPass
	if ( isEncryptedMatching(encryptedPass, encryptedPotential, 11) > 0 )
	{
		/// Matches so set deviceOutputPass to the current combination
		printf("UID '%d' Encrypted pass '%s' from combination '%s' matches potential pass = '%s'\n", uid, encryptedPass, potentialPass, encryptedPotential);
		for (int i = 0; i < 4; i++ ) {
			deviceOutputPass[i] = potentialPass[i];
		}
	}
}

/**
	
*/
int main(int argc, char** argv) {
	printf("Josh Shepherd - 1700471\n\n");

	/// Get the encrypted pass to decrypt
	/*
		Test Encrypted Passwords:
		az01 = ccbdwy2253
		aa52 = ccbddb7362
	*/
	
	char* encryptedPass = "ccbddb7362";
	if (argc > 1) {
		encryptedPass = argv[1];
	}
	 
	printf("Pass: '%s'\n", encryptedPass);
	
	// Init alphabet and numbers array to read only use in cuda 
	char cpuAlphabet[26] = { 'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z' };
	char cpuNumbers[10] = { '0', '1', '2', '3', '4', '5', '6' ,'7', '8', '9' };
	
	int sizeOfEncryptedPass = sizeof(char) * 11;
	
	char* gpuAlphabet;
	cudaMalloc( (void**) &gpuAlphabet, sizeof(char) * 26 );
	cudaMemcpy( gpuAlphabet, cpuAlphabet, sizeof(char) * 26, cudaMemcpyHostToDevice );
	
	char* gpuNumbers;
	cudaMalloc( (void**) &gpuNumbers, sizeof(char) * 10 );
	cudaMemcpy( gpuNumbers, cpuNumbers, sizeof(char) * 10, cudaMemcpyHostToDevice );
	
	char* gpuEncryptedPass;
	cudaMalloc( (void**) &gpuEncryptedPass, sizeOfEncryptedPass );
	cudaMemcpy( gpuEncryptedPass, encryptedPass, sizeOfEncryptedPass, cudaMemcpyHostToDevice);
	
	char* gpuOutputPass;
	cudaMalloc( (void**) &gpuOutputPass, sizeOfEncryptedPass );
	
	
	/// Launch cuda threads and await finish
	decryptPass<<< dim3(26, 26, 1), dim3(10, 10, 1) >>>(gpuAlphabet, gpuNumbers, gpuEncryptedPass, gpuOutputPass);
	cudaDeviceSynchronize();
	
	
	printf("Finished synchronizing CUDA threads\n");
	
	/// Copy GPU output pass to the CPU
	char* cpuOutputPass = (char*)malloc( sizeof(char) * 4 );
	cudaMemcpy(cpuOutputPass, gpuOutputPass, sizeOfEncryptedPass, cudaMemcpyDeviceToHost);

	/// If output pass contained an output, print the results
	printf("---\n");
	printf("Results:\n");
	if (cpuOutputPass != NULL && cpuOutputPass[0] != 0) {
		printf("Given Encrypted Pass: '%s'\n", encryptedPass);
		printf("Found Decrypted Pass: '%s'\n", cpuOutputPass);
	} else {
		printf("Unable to determine a password.\n");
	}
	
	/// Free any malloc'd memory
	cudaFree(gpuAlphabet);
	cudaFree(gpuNumbers);
	cudaFree(gpuEncryptedPass);
	cudaFree(gpuOutputPass);
	free(cpuOutputPass);
}


