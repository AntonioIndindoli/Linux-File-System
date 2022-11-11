/**************************************************************
* Class:  CSC-415-02 Fall 2022
* Names: Evan Giampaoli, Nathan Rennacker, Hann Zhao, Antonio Indindoli 
* Student IDs: 922361476, 921348958, 917565433, 920356733
* GitHub Name: hazhao33
* Group Name: EHNA
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "bitMap.h"
#include "fsLow.h"
#include "mfs.h"
#include "directory.h"

#define MAGIC_NUMBER      0x45484E41
#define VCB_LOCATION      0

// volume control block
struct VCB {
	int numBlocks;      // number of blocks in our volume
	int sizeBlocks;     // size of each block in our volume
	int bitmapLocation; // block index of our freespace map
	int rootLocation;   // block index number of where the root directory starts
	long magicNumber;   // unique signature
};

// Formats our volume
void formatVolume( struct VCB* buffer, uint64_t numberOfBlocks, uint64_t blockSize ) {

	// initialize values of the volume control block
	buffer->numBlocks = numberOfBlocks; 
	buffer->sizeBlocks = blockSize;
	buffer->bitmapLocation = initMap();
	allocateBlocks(1); //allocates first block for VCB
	buffer->rootLocation = initDirectory( NULL );
	buffer->magicNumber = MAGIC_NUMBER;	

	// write newly initialized VCB to block 0
	LBAwrite( buffer, 1, VCB_LOCATION );
}

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize) {
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	
	struct VCB* buffer = malloc( blockSize );
	if (buffer == NULL) {
		fprintf( stderr, "Memory Allocation Error\n" );
		return -1;
	}

	LBAread( buffer, 1, VCB_LOCATION );
	if (buffer->magicNumber != MAGIC_NUMBER) {
		formatVolume( buffer, numberOfBlocks, blockSize );
	}

	cwd = (directoryEntry*) malloc (2369);
	LBAread (cwd, 5, 6); 
	
	free( buffer );
	buffer = NULL;

	return 0;
}
	
	
void exitFileSystem () {
	printf ("System exiting\n");
}