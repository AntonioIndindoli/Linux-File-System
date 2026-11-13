/**************************************************************
* Class:  CSC-415-02 Fall 2022
* Names: Evan Giampaoli, Nathan Rennacker, Hann Zhao, Antonio Indindoli 
* Student IDs: 922361476, 921348958, 917565433, 920356733
* GitHub Name: hazhao33
* Group Name: EHNA
* Project: Basic File System
*
* File: directory.c
*
* Description: Directory Entry File
*
* Inits the root directory and writes it to disk
*
**************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "fsLow.h"
#include "mfs.h"

#include "directory.h"
#include "bitMap.h"
#include "volumeControlBlock.h"

void initDirEntry(directoryEntry *buffer, int i) {
  buffer[i].ext[0].blockNumber = -1;
  buffer[i].ext[0].count = 0;
  buffer[i].ext[1].count = 0;
  buffer[i].ext[2].count = 0;
  buffer[i].fileSize = -1;
}

// TODO eventually convert to extents non-contiguous allocation
int initDirectory (directoryEntry* parent) {
  int startingBlock = 0;
  char dot[] = ".";
  char parentName[] = "..";
  char date[12];

  directoryEntry* buffer = malloc(  INIT_DIR_SIZE * vcb->sizeBlocks );
  if (buffer == NULL) {
    fprintf( stderr, "Memory Allocation Error\n" );
		return -1;
  }
  
  // initialize '.' directory entry
  // pointer to itself
  // memcpy(buffer[0].name, dot, sizeof(dot));
  memset( buffer[0].name, '\0', sizeof( buffer[0].name ) );
  strcpy( buffer[0].name, "." );
  buffer[0].fileSize = sizeof( directoryEntry ) * INIT_DIR_NUMBER;
  buffer[0].isDirectory = 1;
  buffer[0].creationDate = time(NULL);
  buffer[0].modificationDate = time(NULL);
  buffer[0].accessDate = time(NULL);

  // check if need to initialize the root directory or a regular directory
  if (parent == NULL) { 
    // initialize the root directory since parent is NULL

    // initialize '..' directory
    // since this is the root directory and it has no parent
    // '..' directory points to itself 
    buffer[0].ext[0].blockNumber = ROOT_DIR_LOCATION;
    buffer[0].ext[0].count = 1;
    buffer[1].ext[0].blockNumber = ROOT_DIR_LOCATION;
    buffer[1].ext[0].count = 1;

    buffer[1].fileSize = buffer[0].fileSize;
    buffer[1].isDirectory = buffer[0].isDirectory;
    buffer[1].creationDate = time(NULL);
    buffer[1].modificationDate = time(NULL);
    buffer[1].accessDate = time(NULL);
    startingBlock = ROOT_DIR_LOCATION;
  }
  else {   
    // initialize regular directory 

    // printf( "file parent %d\n", parent->fileSize );
    // get a free starting block size 5
    // buffer[0].location = tempSartingBlock;

    // initialize '..' directory
    // pointer to its parent
    //? could change to : buffer[1] = parent[0]?
    buffer[1].ext[0].blockNumber = parent[0].location;
    buffer[1].ext[0].count = parent[0].location;
    buffer[1].fileSize = parent[0].fileSize;
    buffer[1].isDirectory =  parent[0].isDirectory;
    buffer[1].creationDate = parent[0].creationDate;
    buffer[1].modificationDate = parent[0].modificationDate;
    buffer[1].accessDate = parent[0].accessDate;

    // printf( "b1 %d\n", buffer[1].fileSize ); 
    // printf( "b1 %d\n", buffer[1].location ); 
    // printf( "b1 %s\n", buffer[0].name ); 
  }
  
  // memcpy(buffer[1].name, parentName, strlen( parentName ) + 1);
  memset( buffer[1].name, '\0', sizeof( buffer[1].name ) );
  strcpy( buffer[1].name, ".." );

  // init empty directory entry
  for (int i = 2; i < INIT_DIR_NUMBER; i++) {
    // initDirEntry( buffer, i );
    buffer[i].fileSize = -1;
    buffer[i].ext[0].blockNumber = -1;
    buffer[i].ext[0].count = 0;
    buffer[i].ext[1].count = 0;
    buffer[i].ext[2].count = 0;
  }

  startingBlock = allocateBlocks( INIT_DIR_SIZE );
  buffer[0].ext[0].blockNumber = startingBlock;
  buffer[0].ext[0].count = INIT_DIR_SIZE;
  LBAwrite( buffer, INIT_DIR_SIZE, startingBlock);
  // LBAwrite( bitMap_p->map, 5, 1 );
  LBAwrite( bitMap_p, 5, 1 );

  free( buffer );
	buffer = NULL;

  return startingBlock;
}

int getEmptyDirEntry( directoryEntry* buffer ) {

    int numEntries = buffer[0].fileSize / sizeof( directoryEntry );

    for (int i = 2; i < numEntries; i++) {

        // printf( "%d fs %d\n", i, buffer[i].fileSize );
        // printf( "%d fs %s\n", i, buffer[i].name );
        if (buffer[i].fileSize == -1) {
            return i;
        }
    }

    return -1;
}

// void addDirEntry( directoryEntry* buffer, const char* pathname, int newDirIndex ) {
//     buffer = loadDirectory( buffer[newDirIndex] );
//     int lbaPosition = initDirectory( buffer );

//     // strcat(buffer[newDirIndex].name,pathname);

//     // buffer[newDirIndex].location = newDirLoc;
//     // buffer[newDirIndex].fileSize = sizeof( directoryEntry ) * INIT_DIR_NUMBER;
//     // buffer[newDirIndex].isDirectory = 1;
//     // buffer[newDirIndex].creationDate = time(NULL);
//     // buffer[newDirIndex].modificationDate = time(NULL);
//     // buffer[newDirIndex].accessDate = time(NULL);

//     // int numBlocks = ( buffer[newDirIndex].fileSize + (vcb->sizeBlocks - 1)) / vcb->sizeBlocks;

//     // write the new directory onto disk
//     // LBAwrite( buffer, numBlocks, lbaPosition);  
// }


directoryEntry* fs_makeFile( directoryEntry* parent, char* pathname ) {
  // printf( "here\n" );

    int newDirIndex = getEmptyDirEntry( parent );
    if (newDirIndex == -1) {      
        fprintf( stderr, "Directory is full\n" );
        fprintf( stderr, "TODO : expand directorys\n" );

        return NULL;
    }
 
    char copyName[strlen( pathname ) + 1];
    strcpy( copyName, pathname );

    int pos = 0;
    for (int i = strlen( copyName ); i >= 0; --i) {
      if (copyName[i] == '/') {
          pos = i; 
          pos++;
          break;
      }
    }


    int defaultFileSize = 1; // default 1 blocks
    strncpy( parent[newDirIndex].name, pathname + pos, sizeof( parent[newDirIndex].name ) );
    parent[newDirIndex].ext[0].blockNumber = allocateBlocks( defaultFileSize );
    parent[newDirIndex].ext[0].count = defaultFileSize;
    parent[newDirIndex].fileSize = 0;
    parent[newDirIndex].isDirectory = 0;
    parent[newDirIndex].creationDate = time( NULL );
    parent[newDirIndex].modificationDate = time( NULL );
    parent[newDirIndex].accessDate = time( NULL );

   
    int numBlocks = ( parent[0].fileSize + ( vcb->sizeBlocks - 1 )) / vcb->sizeBlocks;
    LBAwrite( parent, numBlocks, parent[0].ext[0].blockNumber ); 
    
    directoryEntry* buffer = loadDirectory( parent[newDirIndex] );

    return buffer;
}

directoryEntry* loadDirectory( directoryEntry entry ) {
  int lbaLocation = entry.ext[0].blockNumber; 
  
  int numBlocks = ( entry.fileSize + ( vcb->sizeBlocks - 1 )) / vcb->sizeBlocks;
  if (numBlocks == 0) {
    numBlocks = 1;
  }

  directoryEntry* buffer = malloc( numBlocks * vcb->sizeBlocks );
  if (buffer == NULL) {
    fprintf( stderr, "Memory Allocation Error : loadDirectory()\n" );

    exit( EXIT_FAILURE );
  }
  
  LBAread( buffer, numBlocks, lbaLocation);

  return buffer;
}
/*
int expandDirSize (directoryEntry *entry) {     //expand 1 block
  int blockCount = (entry->fileSize+511)/BLOCK_SIZE;
  int entryCount = entry->fileSize/DIR_ENTRY_SIZE;
  int newEntryCount = (blockCount+1)/DIR_ENTRY_SIZE;

  directoryEntry *buffer = (directoryEntry *) malloc ((blockCount+1) * BLOCK_SIZE);
  LBAread(buffer, blockCount, entry->location);
  for (int i=entryCount; i<newEntryCount+1; i++) {
    initDirEntry(buffer, i);
  }
  //update parent dir entry
  //update other dir entry
  //get another block
  LBAwrite(buffer,blockCount+1, entry->location);
  //update cwd
  return 0; 
}
*/