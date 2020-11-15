// Josh Shepherd 1700471
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <crypt.h>

struct ThreadArgs {
    int threadIndex;
    
    int firstStartChar;
    int firstEndChar;
    
    int secondStartChar;
    int secondEndChar;
    
    int numStart;
    int numEnd;
    
    char* encryptedPass;
};

/// Global variables
// Found pass determine by a thread once found
char* FoundPass;

void* Decrypt(void* tArg);
void substr(char *dest, char *src, int start, int length);

/**
    Decrypts an encrypted SHA512 password in the format "LetterLetterNumberNumber"
    Arguments:
    First: encrypted password
*/
int main(int argc, char* argv[]) {
    /// Set encrypted pass and parse if passed as argument
    /* 
    	Encrypted passwords for testing
    	AA00 = $6$AS$wKDMKDtx/s3ILNkNaRNFIM0w81/weD1UZ8daNhbQBXuj8L.7OY4trHnSraeizmFYrMwjlb1uRTPxu20rqhmMn/
    	AC05 = $6$AS$xGbvmLMvSO.rAo2XVd7gSXNhyrOv8vNZWcJBQqJxai990EYBzxirCshE2OWfApaSM/dNYaJm7Ttx5VW2slDSj/
	*/
	
    char* encryptedPass = "$6$AS$xGbvmLMvSO.rAo2XVd7gSXNhyrOv8vNZWcJBQqJxai990EYBzxirCshE2OWfApaSM/dNYaJm7Ttx5VW2slDSj/";
    if (argc > 1) {
        encryptedPass = argv[1];
    }

	/// Store pthreads in own array
    int threadCount = 8;
    pthread_t* threadIds = malloc( sizeof(pthread_t) * threadCount);

    /// Iterate over each thread and configure args and create
    for (int i = 0; i < threadCount; i++) {
        struct ThreadArgs* thisArgs = malloc( sizeof(struct ThreadArgs) );
		
		// Set thread index and pass encrypted pass
        thisArgs->threadIndex = i;
        thisArgs->encryptedPass = encryptedPass;
        
        int aChar = 'A';
        int zChar = 'Z';
        int totalCharAmt = (zChar - aChar) + 1; // 26 (characters in alphabet)

        /// How many chars one thread will handle
        int threadCharAmt = totalCharAmt / threadCount;
        
        /// Set search ranges of each thread
        thisArgs->firstStartChar = aChar + (threadCharAmt * i);
        thisArgs->firstEndChar = aChar + (threadCharAmt * i) + (threadCharAmt - 1);
        
        thisArgs->secondStartChar = aChar;
        thisArgs->secondEndChar = zChar;

        thisArgs->numStart = 0;
        thisArgs->numEnd = 99;
    
        //printf("T'%d' Args 1: '%c''%c' 2: '%c''%c' Num: '%d''%d'\n", i, thisArgs->firstStartChar, thisArgs->firstEndChar, thisArgs->secondStartChar, thisArgs->secondEndChar, thisArgs->numStart, thisArgs->numEnd);
    
    	/// Create the thread inside the threadIds array
        pthread_create( &threadIds[i], NULL, Decrypt, thisArgs);
    }
    
    /// Wait for threads to finish
    for(int i = 0; i < threadCount; i++) {
        pthread_join( threadIds[i], NULL );
    }
    
    /// Print out start encrypted and final decrypted pass
    //printf("Completed all threads\n");
    if (FoundPass != NULL) {
    	printf("---\nResults:\n");
    	printf("Encrypted Password: '%s'\n", encryptedPass);
        printf("Decrpyed Password: '%s'\n", FoundPass);
    }
    
    // Free any malloc before exit
    free(threadIds);
    if(FoundPass != null)
    	free(FoundPass);
}

/**
	Function to split decrypting a password over threads using struct ThreadArgs
*/
void* Decrypt(void* tArg) {
    struct ThreadArgs* args = (struct ThreadArgs*)tArg;    
   
    /// Explode the original SHA512 pass into its parts    
    char salt[7];
    substr(salt, args->encryptedPass, 0, 6);
    	
    // Max potential is 'zz99' so 4 chars can handle
    char potentialPass[7]; 
    // Store current iteration encrypted
    char* encrypted;
    
    // Init crypt_data for thread safe crypt function
	struct crypt_data data;
	data.initialized = 0;
    
    for ( int i = args->firstStartChar; i <= args->firstEndChar; i++ ) {
    	for ( int j = args->secondStartChar; j <= args->secondEndChar; j++ ) {
    		for( int k = args->numStart; k <= args->numEnd; k++ ) {
				
				// Exit if another thread found the answer
				if (FoundPass != NULL) {
					pthread_exit(NULL);
				}
				
				/// Create string of potential pass from current iteration
  				sprintf(potentialPass, "%c%c%02d", i, j, k); 
    			
    			/// Encrypt potential password withc the original encrypted pass salt
    			encrypted = (char *) crypt_r(potentialPass, salt, &data);
    
    			/// Print debug/info messages
    			printf("T'%d' Pass: '%s' '%s'\n", args->threadIndex, potentialPass, encrypted);
    
    			/// Check if current encrypted matches the given encrypted pass
    			if ( strcmp( args->encryptedPass, encrypted ) == 0 ) {
    				printf( "T'%d' determined password to be '%s'!\n", args->threadIndex, potentialPass );
    				
    				/// Malloc FoundPass to same size as potentialPass
    				/// Copy and exit
    				FoundPass = malloc (sizeof(potentialPass));
    				strcpy(FoundPass, potentialPass);
    				pthread_exit(NULL);
    				
    				return NULL;
    			}
    		}
    	}
    }
}

/**
	Creates a substring of a string from start index, to the length amount
*/
void substr(char *dest, char *src, int start, int length){
  memcpy(dest, src + start, length);
  *(dest + length) = '\0';
}
