#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <errno.h>
#include <getopt.h>

typedef enum {
	root,
	notRoot
	} rootOrNot;
	
void setSubCommArray(int* subCommArray, int sizeOfSubComm, int rank); 	

int main(int argc, char** argv, char** envp){

	int rank, numProc, i, sizeOfSubComm = 10;
	int numNodes = atoi(getenv("PBS_NUM_NODES"));	
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProc);
	MPI_Group worldGroup, subGroup;
	MPI_Comm subComm;

 struct option long_options[] = {
                {"filename", required_argument, 0, 'f' },
                {NULL,0,0,0}
        };

        int long_index = 0;
        int opt = 0;
	char filename[50];


        while(( opt = getopt_long(argc, argv,"f:",long_options, &long_index)) != -1){
                switch(opt){
                        case 'f' :
				strcpy(filename, optarg);
				break;
		}
	}	
	
	rootOrNot isRoot = notRoot;

	char* buf;
	int* subCommArray = (int*) malloc(sizeof(int) * sizeOfSubComm);
	
	setSubCommArray(subCommArray, sizeOfSubComm, rank); 	

	MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
	MPI_Group_incl(worldGroup, sizeOfSubComm, subCommArray, &subGroup);
	MPI_Comm_create(MPI_COMM_WORLD, subGroup, &subComm);
	if(subComm == MPI_COMM_NULL){
		printf("Ya done messed up\n");
		return 0;
	}

	if(rank % 10 == 0){
		isRoot = root;
	}
	

	int libraryFileSize;
	if(isRoot == root){
		FILE* libraryFile = fopen(filename, "r");
		if(libraryFile == NULL){
			printf("Library file pointer is NULL, see errno: %d\n", errno);
		}
		
		fseek(libraryFile, 0, SEEK_END);
		libraryFileSize = ftell(libraryFile);
		rewind(libraryFile);
	
		buf = (char*) malloc(libraryFileSize);
		
		for(i = 0; i < libraryFileSize; i++){
			buf[i] = i;
		}

		fread(buf, sizeof(char), libraryFileSize, libraryFile);

		fclose(libraryFile);
	}

	MPI_Bcast(&libraryFileSize, 1, MPI_INT, 0, subComm);

	if(isRoot == notRoot){
		buf = (char*) malloc(libraryFileSize);
	}
	
	MPI_Bcast(buf, libraryFileSize, MPI_CHAR, 0, subComm);


	char ramDiskDirectory[100] = "/tmp/scratch/";
	FILE* ramDisk = fopen(strcat(ramDiskDirectory,filename), "w");
	if(ramDisk == NULL){
		printf("RAMDISK file pointer is NULL, see errno: %d\n", errno);
	}
	
	fwrite(buf, sizeof(char), libraryFileSize, ramDisk);
	
	fclose(ramDisk);
	ramDisk = NULL; 

	free(buf);
	free(subCommArray);
	MPI_Finalize();
	
	return 0;


}

void setSubCommArray(int* subCommArray, int sizeOfSubComm, int rank){
	int i;
	int groupID = rank / sizeOfSubComm;
	int rootOfGroup = sizeOfSubComm * groupID;
	for(i = 0; i < sizeOfSubComm; i++){
		subCommArray[i] = rootOfGroup + i;
	}
}







