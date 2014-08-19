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
	char* libFile;
	int* subCommArray;
	int numNodesInNormalComm;
	int numNormalComms;
	int numNodesInSmallComm;
	int libraryFileSize;
	int sizeOfSubComm;
	} FileAndMPI;
	
void setSubCommArray(FileAndMPI* info, int numNodes, int rank); 	

int main(int argc, char** argv, char** envp){

	int rank, numProc, i;
	int numNodes = atoi(getenv("PBS_NUM_NODES"));	
	char filename[50];
	RootOrNot isRoot = notRoot;
	FileAndMPI info;
	
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
	
	while(( opt = getopt_long(argc, argv,"f:",long_options, &long_index)) != -1){
		switch(opt){
			case 'f' :
				strcpy(filename, optarg);
				break;
		}
	}		
	setSubCommArray(&info, numNodes, rank); 	

	MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
	MPI_Group_incl(worldGroup, info.sizeOfSubComm, info.subCommArray, &subGroup);
	MPI_Comm_create(MPI_COMM_WORLD, subGroup, &subComm);

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
	
		info.libFile = (char*) malloc(info.libraryFileSize);
		
		for(i = 0; i < info.libraryFileSize; i++){
			info.libFile[i] = i;
		}

		fread(info.libFile, sizeof(char), info.libraryFileSize, libraryFile);
		fclose(libraryFile);
	}

	MPI_Bcast(&(info.libraryFileSize), 1, MPI_INT, 0, subComm);
	if(isRoot == notRoot){
		info.libFile = (char*) malloc(info.libraryFileSize);
	}
	MPI_Bcast(info.libFile, info.libraryFileSize, MPI_CHAR, 0, subComm);


	char ramDiskDirectory[100] = "/tmp/scratch/";
	FILE* ramDisk = fopen(strcat(ramDiskDirectory,filename), "w");
	if(ramDisk == NULL){
		printf("RAMDISK file pointer is NULL, see errno: %d\n", errno);
	}
	
	fwrite(info.libFile, sizeof(char), info.libraryFileSize, ramDisk);
	fclose(ramDisk);

	free(info.libFile);
	free(info.subCommArray);
	MPI_Finalize();
	
	return 0;
}

void setSubCommArray(FileAndMPI* info, int numNodes, int rank){
	info->numNodesInNormalComm = 10;
	info->numNormalComms = numNodes / info->numNodesInNormalComm;
	info->numNodesInSmallComm = numNodes - (info->numNodesInNormalComm * info->numNormalComms);	
	int i;
	int groupID = rank / info->numNodesInNormalComm;
	int rootOfGroup = info->numNodesInNormalComm * groupID;

	if(groupID < info->numNormalComms){
		info->subCommArray = (int*) malloc(sizeof(int) * info->numNodesInNormalComm);

		for(i = 0; i < info->numNodesInNormalComm; i++){
			info->subCommArray[i] = rootOfGroup + i;
		}
		info->sizeOfSubComm = info->numNodesInNormalComm;
	}else{
		info->subCommArray = (int*) malloc(sizeof(int) * info->numNodesInSmallComm);
		
		for(i = 0; i < info->numNodesInSmallComm; i++){
			info->subCommArray[i] = rootOfGroup + i;
		}
		info->sizeOfSubComm = info->numNodesInSmallComm;
	}
}

	
