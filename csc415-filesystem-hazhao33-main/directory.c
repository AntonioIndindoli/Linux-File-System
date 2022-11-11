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

#include "fsLow.h"
#include "mfs.h"

#include "directory.h"
#include "bitMap.h"

#define ROOT_DIR_LOCATION 6
#define INIT_DIR_NUMBER 8
#define DIR_ENTRY_SIZE 296
#define INIT_DIR_SIZE 5   //8 dir entry = 2432 byte = 5 block
//INIT_DIR_SIZE 2432 byte
//fill in directory entry
void initDirEntry(directoryEntry *buffer, int i) {
  buffer[i].location = -1;
  buffer[i].fileSize = -1;
}

//create directory return starting block number
int initDirectory (directoryEntry *parent) {
  int startingBlock = 0;
  char dot[] = ".";
  char parentName[] = "..";
  char date[12];
  directoryEntry *buffer = (directoryEntry *) malloc (INIT_DIR_SIZE*512);
  if (buffer == NULL) {
    fprintf( stderr, "Memory Allocation Error\n" );
		return -1;
  }
  // init directory
  if (parent == NULL) { //create root directory
    //adding directory entry for .
    buffer[0].location = ROOT_DIR_LOCATION;
    //adding directory entry for ..
    buffer[1].location = ROOT_DIR_LOCATION;
    buffer[1].fileSize = buffer[0].fileSize;
    buffer[1].isDirectory = buffer[0].isDirectory;
    buffer[1].creationDate = time(NULL);
    buffer[1].modificationDate = time(NULL);
    buffer[1].accessDate = time(NULL);
    startingBlock = ROOT_DIR_LOCATION;
  }else { //create normal directory  
    //self directory entry .
    //get a free starting block size 5
    int tempSartingBlock = 10;
    buffer[0].location = tempSartingBlock;
    //adding directory entry for parent ..
    buffer[1].location = parent->location;
    buffer[1].fileSize = parent->fileSize;
    buffer[1].isDirectory =  parent->isDirectory;
    buffer[1].creationDate = parent->creationDate;
    buffer[1].modificationDate = parent->modificationDate;
    buffer[1].accessDate = parent->accessDate;
    startingBlock = tempSartingBlock;
  }
  memcpy(buffer[0].name, dot, sizeof(dot));
  buffer[0].fileSize = DIR_ENTRY_SIZE * INIT_DIR_NUMBER;
  buffer[0].isDirectory = 1;
  buffer[0].creationDate = time(NULL);
  buffer[0].modificationDate = time(NULL);
  buffer[0].accessDate = time(NULL);
  memcpy(buffer[1].name, parentName, sizeof(parentName));
  //init empty directory entry
  for (int i=2; i<INIT_DIR_NUMBER; i++) {
    initDirEntry(buffer, i);
  }
  startingBlock = allocateBlocks(INIT_DIR_SIZE);
  LBAwrite(buffer, INIT_DIR_SIZE, startingBlock);
  free(buffer);
	buffer = NULL;
  return startingBlock;
}

