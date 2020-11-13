// Josh Shepherd 1700471
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

struct SHAExploded {
	char* salt;
	char* second;
	char* plain;
	char* checksum;
};

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
char* GetSHAPassSalt(char* sha512);

/**
    Decrypts an encrypted SHA512 password in the format "LetterLetterNumberNumber"
    Arguments:
    First: encrypted password
*/
int main(int argc, char* argv[]) {
    /// Set encrypted pass and parse if passed as argument
    char* encryptedPass = "$6$AS$.PM.QsfUIFitffTV3mHCVh4XhzVPlFpS5GVAw/5RzfC6PSNZWBreqGKFrY28PqXvJ775zWZBuX57FR.efob6j1"; //aa11 encrypted SHA512
    if (argc > 1) {
        encryptedPass = argv[1];
    }

	/// Store pthreads in own array
    int threadCount = 2;
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
    
        printf("Thread '%d' handles 1st char '%c' to '%c'\n", i, thisArgs->firstStartChar, thisArgs->firstEndChar);
        printf("Thread '%d' handles 2nd char '%c' to '%c'\n", i, thisArgs->secondStartChar, thisArgs->secondEndChar);
    
        pthread_create( &threadIds[i], NULL, Decrypt, thisArgs);
    }
    
    /// Wait for threads to finish
    for(int i = 0; i < threadCount; i++) {
        pthread_join( threadIds[i], NULL );
    }
    
    printf("Complete!\n");
    if (FoundPass != NULL) {
        printf("FoundPass = '%s'\n", FoundPass);
    }
    
    // Free any malloc before exit
    free(threadIds);
}

void* Decrypt(void* tArg) {
    struct ThreadArgs* args = (struct ThreadArgs*)tArg;    
   
    /// Explode the original SHA512 pass into its parts    
	char* salt = GetSHAPassSalt(args->encryptedPass);
	printf("Salt: %s\n", salt);
    
    // Max potential is 'zz99' so 4 chars can handle
    char potentialPass[15]; 
    // Store current iteration encrypted
    char* encrypted;
    
    for ( int i = args->firstStartChar; i <= args->firstEndChar; i++ ) {
    	for ( int j = args->secondStartChar; j <= args->secondEndChar; j++ ) {
    		for( int k = args->numStart; k <= args->numEnd; k++ ) {
				
				// Exit if another thread found the answer
				if (FoundPass != NULL) {
					pthread_exit(NULL);
				}
				
				/// Create string of potential pass from current iteration
    			sprintf( potentialPass, "%c%c%02d", i, j, k );
    			
    			printf("Thread '%d' checking '%s' with salt '%s'\n", args->threadIndex, potentialPass, salt);
    			
    			/// Encrypt potential password with the original encrypted pass salt
    			encrypted = (char*) crypt(potentialPass, salt);
    
    			/// Check if current encrypted matches the given encrypted pass
    			if ( strcmp( args->encryptedPass, encrypted ) == 0 ) {
    				printf( "Password is '%s'!", potentialPass );
    				
    				/// Set the pass and exit the thread
    				FoundPass = potentialPass;
    				pthread_exit(NULL);
    				
    				return NULL;
    			}
    			
    			if (k > 0) {
	    			return NULL;
    			}
    		}
    	}
    }
}

/**
	Explodes a SHA512 encrypted password into it's parts
	and returns a struct of each part
*/
char* GetSHAPassSalt(char* sha512) {
	char* salt;
	
	char shaCpy[512];
	strcpy(shaCpy, sha512);
	
	int fullStopCount = 0;
	
	char* split = strtok(shaCpy, ".");
	
	if (split != NULL) {
		salt = split;
	}
	
	return salt;
}

