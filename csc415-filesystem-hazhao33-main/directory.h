/**************************************************************
* Class:  CSC-415-02 Fall 2022
* Names: Evan Giampaoli, Nathan Rennacker, Hann Zhao, Antonio Indindoli 
* Student IDs: 922361476, 921348958, 917565433, 920356733
* GitHub Name: hazhao33
* Group Name: EHNA
* Project: Basic File System
*
* File: directory.h
*
* Description: Directory Header File
*
*
**************************************************************/

//directory Entry and file metadate for our system
typedef struct directoryEntry {
  char name[256];           //file name
  int location;          //starting block location
  int fileSize;             //file size in byte
  int isDirectory;          //flag  directory = 1   file = 0
  // directory access tracker
  time_t creationDate;
  time_t modificationDate;
  time_t accessDate;
}directoryEntry;

void initDirEntry(directoryEntry *buffer, int i);

int initDirectory (directoryEntry *parent);
