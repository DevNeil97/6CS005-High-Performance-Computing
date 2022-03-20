/**
In this task, you will be asked to use the “crypt” library to decrypt a password using multithreading.
You will be provided with two programs. The first program called “EncryptSHA512.c” allows you to encrypt 
a password. For this assessment, you will be required to decrypt a 4-character password consisting of 2 
capital letters, and 2 numbers. The format of the password should be “LetterLetterNumberNumber.” 
For example, “HP93.” Once you have generated your password, this should then be entered into your program 
to decrypt the password. The method of input for the encrypted password is up to you. The second program is 
a skeleton code to crack the password on a single thread without any multithreading syntax. Your task is to 
use the pthread or omp library to split the workload over many threads and find the password. Once the password 
has been found, the program should finish meaning not all combinations of 2 letters and 2 numbers should be 
explored unless it’s ZZ99 AND the last thread happens to finish last.

./a.out numOfThreads  encryptedPassword(Optional)

Created By Nirmal Abeykoon Mudiyanselage 1811342
6CS005 - Assessment 21-22
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <crypt.h>

/**
*Thread info Struct
*/
typedef struct $ {
    int rank;
    int start;
    int end;
    char* hash;
}threadinfo;

//Globel Variabels and Methods
char *pPass;
threadinfo *ti;
long testval=0;
pthread_mutex_t lock;
int loopcount = 0;

//Methods
void *threadRunner (void *rank);
void substr(char *dest, char *src, int start, int length);

/**
 * Main
 * */
int main (int argc, char* argv []){

	char* hash = "$6$AS$Ov2MJQpAfz1tapBAvIO3DrK.6rSd5E8abPPezB2PXQRBrW/.ZKnEIzyeKWeJCSz1ls/vunYlV8Jda94BDjJ/t/";
	
    	/**
    	*User must enter atleast the number of threads to use
    	*/
    	if (argc <= 1) {
			printf("Usage%s Required Number of threads\n",argv[0] );
			exit(-1);
    	}

	/**
     	* If user enters a hash in arguments use that insted of the defult value 
     	* */  	
    //getting the number of threads and allocating memory
    int threadnum = atoll(argv[1]);
	ti=malloc((sizeof(threadinfo)*threadnum));

	//split the workload equaly
	//if there is more than 26 thread set the max amount to 26
	if (threadnum > 26){
		threadnum = 26;
		printf("Max thread Number reached.Setting the threadNum value to 26\n");
	}

	//spred the workload
	long long combos = 26;
	int reminder = combos%threadnum;
	printf("Number of possible Combos %lli\n",combos);
	long chunkSize = combos/threadnum;

	//split the workload equaly
	/*
	ti[0].start = 0;
	ti[0].end = 13;
	ti[1].start = 13;
	ti[1].end = 26;
	*/
	int counter = 0;
	for(int i = 0; i < threadnum; i++){
		if (i == 0){
			ti[i].start = 0;
			ti[i].end = chunkSize;
			continue;
		}
		if (i == (threadnum-1)){
			ti[i].start=ti[i-1].end;
			ti[i].end = combos;
			break;
		}
			ti[i].start=ti[i-1].end;
			ti[i].end = chunkSize+ti[i].start;
	}
	while(!reminder == 0){
		ti[counter].end++;
		ti[counter+1].start++;
		counter++;
		reminder--;
	}
	//printf("chunksize = %ld rem=%d\n" , chunkSize,reminder);
	//Create Threads
	pthread_t thread_ids[threadnum];
	for (int i = 0; i < threadnum; ++i){
		ti[i].rank=i;
		ti[i].hash=hash;
		
		//Print each thread start and end point
		printf("Thread Rank -->%d Thread Start-->%d  Thread End-->%d\n"
			,ti[i].rank,ti[i].start,ti[i].end);
		//create threads
		int returnValue = pthread_create(&thread_ids[i],NULL,
		threadRunner,(void*)&ti[i].rank);
		
		//error check
		if(returnValue!=0){
			printf("Error occured while creating threads. value=%d\n",returnValue );
		}
	}
	
	//Join Threads
	for (int i = 0; i< threadnum; i++){
		pthread_join(thread_ids[i],NULL);
	}

	//print details
	if(pPass!=NULL){
		printf("decrypted password----->%s\n",pPass);
	}else{
		printf("Error Decrypting the password.Please try again\n");
	}
    
	//Free Memory
    free(ti);
    pthread_mutex_destroy(&lock);
	//printf("%d\n",loopcount);
    return 0;
}


/**
 * Thread Runner
 * */
void *threadRunner (void *rank){
	int localRank = *(int*) rank;
	char plain[7];
	char *enc;
	char salt[7];
	
	substr(salt, ti[localRank].hash, 0, 6);
	
	//crypt_data for thread safe function
	struct crypt_data data;
	data.initialized = 0;
	
	//printf("Hello From %d\nHash=%s\n",rank,hash);
	for (int i = 'A'+ti[localRank].start; i < 'A'+ti[localRank].end; i++ ){
		for (int j = 'A'; j <= 'Z'; j++){
			for(int z=0; z<=99; z++){
					
				if (pPass!=NULL){
					pthread_exit(NULL);
				}
					
				//create a protential password
				sprintf(plain, "%c%c%02d", i, j, z);
					
				//encrypt protential pass with current hashed pass
				enc = (char *) crypt_r(plain, salt, &data);
				if (strcmp(enc,ti[localRank].hash)==0){
					//allocating memmory for found pass
					pPass = malloc (sizeof(plain));
					//save pass from plain to found pass
					strcpy(pPass, plain);
					//terminate pass
					printf("%s\n",plain);
					pthread_exit(NULL);
				}
				if (pPass!=NULL){
					pthread_exit(NULL);
				}
				printf("Thread%d ---> %s\n",localRank,plain);
			}
		}
	}
}

void substr(char *dest, char *src, int start, int length){
  memcpy(dest, src + start, length);
  *(dest + length) = '\0';
}

