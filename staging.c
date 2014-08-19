#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <errno.h>
#include <getopt.h>

typedef enum {
	root,
	notRoot
	} RootOrNot;
typedef struct {
	int* subCommArray;
	int sizeOfSubComm;
	char* buf;
	int libraryFileSize;
	} FileAndMPI;
	
void setSubCommArray(FileAndMPI* info, int rank); 	

int main(int argc, char** argv, char** envp){

	int rank, numProc, i;
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
	
	RootOrNot isRoot = notRoot;
	FileAndMPI info;

	info.sizeOfSubComm = 10;
	info.subCommArray = (int*) malloc(sizeof(int) * info.sizeOfSubComm);
		
	setSubCommArray(&info, rank); 	

	MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
	MPI_Group_incl(worldGroup, info.sizeOfSubComm, info.subCommArray, &subGroup);
	MPI_Comm_create(MPI_COMM_WORLD, subGroup, &subComm);
	if(subComm == MPI_COMM_NULL){
		printf("Ya done messed up\n");
		return 0;
	}

	if(rank % 10 == 0){
		isRoot = root;
	}
	
	if(isRoot == root){
		FILE* libraryFile = fopen(filename, "r");
		if(libraryFile == NULL){
			printf("Library file pointer is NULL, see errno: %d\n", errno);
		}
		
		fseek(libraryFile, 0, SEEK_END);
		info.libraryFileSize = ftell(libraryFile);
		rewind(libraryFile);
	
		info.buf = (char*) malloc(info.libraryFileSize);
		
		for(i = 0; i < info.libraryFileSize; i++){
			info.buf[i] = i;
		}

		fread(info.buf, sizeof(char), info.libraryFileSize, libraryFile);

		fclose(libraryFile);
	}

	MPI_Bcast(&(info.libraryFileSize), 1, MPI_INT, 0, subComm);

	if(isRoot == notRoot){
		info.buf = (char*) malloc(info.libraryFileSize);
	}
	
	MPI_Bcast(info.buf, info.libraryFileSize, MPI_CHAR, 0, subComm);


	char ramDiskDirectory[100] = "/tmp/scratch/";
	FILE* ramDisk = fopen(strcat(ramDiskDirectory,filename), "w");
	if(ramDisk == NULL){
		printf("RAMDISK file pointer is NULL, see errno: %d\n", errno);
	}
	
	fwrite(info.buf, sizeof(char), info.libraryFileSize, ramDisk);
	
	fclose(ramDisk);
	ramDisk = NULL; 

	free(info.buf);
	free(info.subCommArray);
	MPI_Finalize();
	
	return 0;


}

void setSubCommArray(FileAndMPI* info, int rank){
	int i;
	int groupID = rank / info->sizeOfSubComm;
	int rootOfGroup = info->sizeOfSubComm * groupID;
	for(i = 0; i < info->sizeOfSubComm; i++){
		info->subCommArray[i] = rootOfGroup + i;
	}
}







