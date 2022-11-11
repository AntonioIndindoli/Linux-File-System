/**************************************************************
* Class:  CSC-415-02 Fall 2022
* Names: Evan Giampaoli, Nathan Rennacker, Hann Zhao, Antonio Indindoli 
* Student IDs: 922361476, 921348958, 917565433, 920356733
* GitHub Name: hazhao33
* Group Name: EHNA
* Project: Basic File System
*
* File: bitMap.h
*
* Description: BitMap header file for include purposes
*
* Contains the bitMap structure as well as the functions:
* initMap(), allocateBlocks(), writeBlocks() (and printMap()--for testing)
*
**************************************************************/
#include <stdint.h>
#define NUM_BLOCKS 19531


/**
 * initilize free space map and write it to the LBA
 * 
 * returns the location of the bitmap in the LBA
 */
int initMap();

/**
 * allocates n (length) blocks in the first free CONTIGUOUS space found
 * 
 * returns the position to where the first block is stored
*/
int allocateBlocks(int length);

/**
 * sets bits from starting block location to length
 * returns -1 if map blockSize is exceeded
 * returns 0 on success
*/
int writeBlocks(int start, int length);

/**
 * TESTING CODE
 * prints the bitMap in form
 * 0000 0000
 * 0000 0000
 * 0000 0000
 * etc.
 * reads right to left
 * 0000 0010 (second bit filled)
*/
void printMap();
