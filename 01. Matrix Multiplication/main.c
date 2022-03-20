/**
1.Matrix Multiplication -->15% - 100 marks
You will create a matrix multiplication program which uses multithreading, you will be taught two 
CPU multithreading concepts: POSIX threads and Open Multi-Processing (OpenMP). Matrices are often 
two-dimensional arrays varying in sizes (can also be 3D), for your application, you will only need 
to multiply two-dimensional ones. Your program will read in the matrix from a file (txt), store them 
appropriately using dynamic memory and multiply them by splitting the tasks across “n” threads 
any number of threads via command line arguments). Each matrix will be in a separate file. You should 
also use command line arguments to allow the user to enter the which matrices need to be multiplied. 
The data files will be given to you however, you will need to initially identify the dimensions of 
the matrix to determine whether they can be multiplied. If the matrices cannot be multiplied, your 
program should notify the user. For example, if Matrix A is 3x3 and Matrix B is 2x2, you cannot 
multiply them. If Matrix A is 2x3 and Matrix B is 2x2, then this can be multiplied. You will need to 
research how to multiply matrices, this will also be covered in the lectures. The resulting matrix 
should be outputted to a file. It is up to you which multithreading library you use (Pthreads or OMP). 
You will be given 10 data files to experiment with but remember, the dimensions need to be detected 
dynamically. During marking, we will be using a different dataset and therefore, do not hard-code 
your program to work with specific dimensions of matrices.

Created By Nirmal Abeykoon Mudiyanselage 1811342
6CS005 - Assessment 21-22

***************Compile help***********************
./a.out numthreads matrixfile1.txt matrixfile2.txt
*************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

// Crerating Structs
/*
*Matrix that contains it's size and values
*/
typedef struct Matrix
{
	int row;//rows of the matrix
	int col;//col of the matrix
	double **values; //all the values of the matrix
} Matrix;

typedef struct ThreadInfo
{
	int rank;
	int start;
	int end;
} ThreadInfo;

//Defining Globel Variabels
ThreadInfo *ti;
Matrix matrixA;
Matrix matrixB;
Matrix matrixC;

//Defining Functions
void *threadRunner(void *rank);
void getSize(FILE *fp,Matrix *matrix);
void readFile(char filename[], Matrix *matrix);
void printMatrix(Matrix *matrix);
int canMultiply(Matrix *matrixA, Matrix *matrixB);
void multiply(double numbers[],int rank);
void saveMatrix(Matrix *matrix);

//main
int main(int argc, char *argv[])
{

	//argument check
	if (argc < 4){
		printf("Usage: %s required 3 args\n",argv[0]);
		exit(-1);
	}

	//getting values from CL arguments
	int threadNum = atoi(argv[1]);
	char *fileA  = argv[2];
	char *fileB  = argv[3];

	//read matrixA from file one
	readFile(fileA, &matrixA);

	//read matrixB from file two
	readFile(fileB, &matrixB);
	
	//Print matrixA
	printf("Matrix A is:\n");
	printMatrix(&matrixA);
	
	//Print matrixB
	printf("\n\nMatrix B is:\n");
	printMatrix(&matrixB);

	if (!canMultiply(&matrixA,&matrixB)){
		
		//set the dementions of matrixC
		matrixC.row=matrixA.row;
		matrixC.col=matrixB.col;

		//malloc mem for matrixC
		matrixC.values = malloc(sizeof(double*)*matrixC.row);
		for (int i = 0; i < matrixC.row; ++i)
		{
			matrixC.values[i]= malloc(sizeof(double)*matrixC.col);
		}

		/*
		// Single thread Multiplication
		for(int i=0;i<matrixA.row;i++){
			for(int j=0;j<matrixA.row;j++){
				for(int k=0;k<matrixA.col;k++){
					matrixC.values[i][j] += matrixA.values[i][k] * matrixB.values[k][j];
				}
			}
		}*/
		

		
		ti=malloc(sizeof(ThreadInfo)*threadNum);

		//Slice the workload
		int chunksize = matrixA.row/threadNum;
    	for (int i = 0; i <threadNum; ++i){
    		ti[i].rank=i;
	    	if (i==0){
	    		ti[i].start = i;
	    		ti[i].end = i+chunksize-1;
	    		continue;
	    	}
	    	if (i == (threadNum-1))
	    	{
	    		ti[i].start = ti[i-1].end+1;
	    		ti[i].end = (matrixA.row-1);
	    		break;
	    	}
	    	ti[i].start = ti[i-1].end+1;
	    	ti[i].end = ti[i].start+chunksize;
	    	
    	}
		
		/*
		for (int i = 0; i < threadNum; ++i)
		{
			printf("Thread-%d start:%d end:%d\n",ti[i].rank,ti[i].start,ti[i].end);
		}
		*/

		//Create threads
		pthread_t thread_ids[threadNum];
		for (int i = 0; i < threadNum; ++i)
		{
			int returnValue = pthread_create(&thread_ids[i],NULL,
				threadRunner,(void*)&ti[i].rank);
			if(returnValue!=0){
				printf("Error occured while creating threads. value=%d\n",returnValue );
			}
		}

		// join treads
		for (int local_rank = 0; local_rank < threadNum; ++local_rank){
			pthread_join(thread_ids[local_rank], NULL);
		}
	
	}else{
		printf("Matrix A and B Cannot be multiplied\n");
		exit(0);
	}

	printf("\n\nmatrix C is:\n");
	printMatrix(&matrixC);
	saveMatrix(&matrixC);
	printf("\n\nResults Saved as Answere.txt\n\n");

	//free memory
	free(ti);
	free(matrixA.values);
	free(matrixB.values);
	free(matrixC.values);
	return 0;
}

/**
 * Read a text file and create a matrix
 * @arg1: filename
 * @arg2: matrix memory buffer
*/
void readFile(char filename[], Matrix *matrix)
{
	FILE* numfile = fopen(filename,"r");
	char *piece;
	int currentRow=0;
	int currentCol=0;
	if (numfile == NULL){
		printf("File Read error: %s\n", 
                         strerror(errno));
               exit(-1);
	}
	getSize(numfile, matrix);
	
	//read file and store matrix values in to 2D array
	char currentLine[1000];
	while(fgets(currentLine,sizeof(currentLine),numfile)){
		piece = strtok(currentLine,",");
		while(piece != NULL){
			matrix->values[currentRow][currentCol]=atof(piece);
			piece = strtok(NULL,",");
			currentCol++;
		}

		currentRow++;
		currentCol = 0;
	}

}

/**
*Get the dementions of a matrix
*@fp - file pointer 
*@matrix - pointer to a matrix
*/
void getSize(FILE *fp,Matrix *matrix){
	// finding number of Rows in the matrix
	char ch;
	matrix->row=1;
	while((ch=fgetc(fp))!=EOF){
		if (ch == '\n'){
			matrix->row++;
		}
	}
	rewind(fp);
	
	// finding number of Cols in the matrix
	matrix->col=1;
	while((ch=fgetc(fp))!='\n'){
		if (ch == ','){
			matrix->col++;
		}
	}
	rewind(fp);
	
	//Allocating memory for matrix values
	matrix->values = malloc(sizeof(double*)*matrix->row);
	for (int i = 0; i < matrix->row; ++i)
	{
		matrix->values[i]= malloc(sizeof(double)*matrix->col);
		//printf("%d",i);
	}
}
/**
*Print matrix to CLI
*/
void printMatrix(Matrix *matrix){
	for(int i=0;i< matrix->row ; i++){
        	for(int j=0;j<matrix->col;j++){
            		printf("%f|", matrix->values[i][j]);
        }
        printf("\n");
    }
}

/**
 * Check if two matricis can be multiplied
 * @matrixA matrixA that needed to be cheacked
 * @matrixA matrixB that needed to be cheacked
 * */

int canMultiply(Matrix *matrixA, Matrix *matrixB){
	if(matrixA->col == matrixB->row){
		return 0;
	}else{
		return 1;
	}
}

/**
 * Save matix data to a File
 * @matrix matrix mem buffer
 * */
void saveMatrix(Matrix *matrix){
	FILE* fp = fopen ("Answer.txt","w");
		for(int i=0;i< matrix->row ; i++){
        	for(int j=0;j<matrix->col;j++){
        		if(j== (matrix->col-1)){
        			fprintf(fp,"%f", matrix->values[i][j]);
        		}else{
            		fprintf(fp,"%f,", matrix->values[i][j]);
        	}
        }
        fprintf(fp,"\n");
    }

}

/**
 * Thread Runner
 * @rank rank of the thread
 * */
void *threadRunner(void *rank){
	int threadRank = *(int*)rank;
	for (int i = ti[threadRank].start; i <=ti[threadRank].end; ++i)
	{
		multiply(matrixA.values[i],i);
	}
}

/**
 * Multiply two matrix values
 * @numbers[] matrix values
 * */

void multiply(double numbers[],int rank){
	for (int i = 0; i < matrixC.col; ++i)
	{
		for (int j = 0; j < matrixA.col; ++j)
		{
			matrixC.values[rank][i]+=matrixA.values[rank][j]*matrixB.values[j][i];

			//printf("%f ---> %f\n",matrixA.values[rank][j],matrixB.values[j][i]);
		}
	}
	
}
