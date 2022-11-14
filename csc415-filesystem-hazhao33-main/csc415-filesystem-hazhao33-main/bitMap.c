/**************************************************************
* Class:  CSC-415-02 Fall 2022
* Names: Evan Giampaoli, Nathan Rennacker, Hann Zhao, Antonio Indindoli 
* Student IDs: 922361476, 921348958, 917565433, 920356733
* GitHub Name: hazhao33
* Group Name: EHNA
* Project: Basic File System
*
* File: bitMap.c
*
* Description: BitMap structure and functions
*
* Includes the functions to find and allocate empty blocks
* as well as clear them. Map is initialized with initMap()
* which fills bitMap values 1,5 (as that's the space required by the bitMap itself)
*
**************************************************************/
#include "bitMap.h"

#include <stdio.h>
#include <stdlib.h>

#include "fsLow.h"

// convert int to bits (in this case it's 4 * 8)
#define BITS_PER_UINT (sizeof(uint32_t) * 8)
// convert function values (block number) for bitwise ops
// (these are pretty amazing, remind me of anonymous functions from higher level languages)
// calculates which integer within the bitMap array corresponds to the block
#define INT_OFFSET(b) ((b) / BITS_PER_UINT)
// uses remainder (modulo) to get which specific bit to work on within 32
// ie we want to get the 1542nd block -> 1542 % 32 = 12, so this means we want the 12th bit within the 32
#define BIT_OFFSET(b) ((b) % BITS_PER_UINT)

// testing code for printing found on w3schools, is pretty hacky and will be removed
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)       \
    (byte & 0x80 ? '1' : '0'),     \
        (byte & 0x40 ? '1' : '0'), \
        (byte & 0x20 ? '1' : '0'), \
        (byte & 0x10 ? '1' : '0'), \
        (byte & 0x08 ? '1' : '0'), \
        (byte & 0x04 ? '1' : '0'), \
        (byte & 0x02 ? '1' : '0'), \
        (byte & 0x01 ? '1' : '0')

typedef struct bitMap{
    //map that stores 19531 bits 
    //(really 19552 but we only have 19531 available bits so the extras are disregarded)
    //need the extra one for the extra bits (need 611 unsigned integers -- not 610)
    uint32_t map[(NUM_BLOCKS/32) + 1]; 
}bitMap;

bitMap* bitMap_p;

/**
 * FORWARD DECLARATION BLOCK
*/

/**
 * clears bits from starting block location to length
 * returns -1 if map blockSize is exceeded
 * returns 0 on success
*/
int clearBlocks(int start, int length);

/**
 * finds n empty bits in sequence *length*
 * returns block position if found
 * returns -1 if sequence cannot be found
*/
int findEmptyBlocks(int length);

/**
 * takes first empty block and checks if each subsequent block is empty 
 *    until length is reached
 * 
 * returns n = length if all blocks were empty otherwise returns n < length
*/
int recurseCount(int bit, int length);

//sets the numbered bit within the map
void setBit(int bit);

//clears the numbered bit within the map
void clearBit(int bit);

//finds the specific bit within the map
int findBit(int bit);

/**
 * FORWARD DECLARATION END
*/

int initMap() {
    bitMap_p = malloc(5 * MINBLOCKSIZE);
    if (bitMap_p == NULL) {
        fprintf( stderr, "Memory Allocation Error" );
        return -1;
    }
    for (int i = 0; i < NUM_BLOCKS - 1; i++) {
        clearBit(i);
    }
    writeBlocks(1, 5); //write 5 blocks for the bitmap's existence and the first block for the VCB
    LBAwrite(bitMap_p->map, 5, 1);

    return 1;
}

int allocateBlocks(int length){
    int blockPos = findEmptyBlocks(length);
    if(blockPos < 0){
        printf("ERROR: no free space found");
        return -1;
    }
    writeBlocks(blockPos, length);
    LBAwrite(bitMap_p->map, 5, 1);
    return blockPos;
}

int writeBlocks(int start, int length) {
    if (start + length > NUM_BLOCKS) {
        printf("Trying to set bits exceeding number of blocks");
        return -1;
    } else {
        for (int i = start; i < (start + length); i++) {
            setBit(i);
        }
        return 0;
    }
}

int clearBlocks(int start, int length) {
    if (start + length > NUM_BLOCKS) {
        printf("Trying to clear bits exceeding number of blocks");
        return -1;
    } else {
        for (int i = start; i < (start + length); i++) {
            clearBit(i);
        }
        return 0;
    }
}

int findEmptyBlocks(int length) {
    for (int i = 0; i < NUM_BLOCKS - 1; i++) {
        if (findBit(i) == 0) {
            if (recurseCount(i, length) == length) {
                return i;
            }
        }
    }
    return -1;
}

int recurseCount(int bit, int length) {
    if (length == 0) {
        return 0;
    } else if (findBit(bit) == 0) {
        return recurseCount((bit + 1), length - 1) + 1;
    } else {
        return -1;
    }
}

void setBit(int bit) {
    // takes the value from the map (with the correct offset for integer)
    // and uses an OR operation to set a specific bit (that corresponds to a block)
    //<< is a left shift operation effectively multiplying bit * 2^1
    bitMap_p->map[INT_OFFSET(bit)] |= ((uint32_t)1 << BIT_OFFSET(bit));
}

void clearBit(int bit) {
    // same as above but uses an AND operation (and one's complement) to clear the bit
    bitMap_p->map[INT_OFFSET(bit)] &= ~((uint32_t)1 << BIT_OFFSET(bit));
}

int findBit(int bit) {
    // checks to see if the specific block in the bit is set
    if ((bitMap_p->map[INT_OFFSET(bit)] & ((uint32_t)1 << BIT_OFFSET(bit)))) {
        return 1;
    } else {
        return 0;
    }
}

// Testing Code to be removed
void printMap() {
    for (int m = 0; m < (NUM_BLOCKS / 32); m++) {
        printf("m: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(bitMap_p->map[m] >> 8), BYTE_TO_BINARY(bitMap_p->map[m]));
    }
    printf("\n");
}
