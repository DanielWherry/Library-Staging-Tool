#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char** argv){

	int rank, numProc, i = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProc);

	int* writeToRamDisk = (int*) malloc(4 * 64);
	
	for( i = 0; i < 64; i++){
		writeToRamDisk[i] = i;
	}

	if(rank == 0 || rank == 16 || rank == 32 || rank == 48){
		
		FILE* ramDisk;	
		printf("Hello\n");
		fopen("/tmp/scratch", "wb");
		printf("nope\n");

		fwrite(writeToRamDisk, 4, 63, ramDisk);
		printf("ya\n");

		fclose(ramDisk);
		ramDisk = NULL; 
		printf("gotcha\n");
	}

	free(writeToRamDisk);
	MPI_Finalize();
	
	return 0;


}

