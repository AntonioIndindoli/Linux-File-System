/**************************************************************
* Class:  CSC-415
* Name: Professor Bierman
* Student ID: N/A
* Project: Basic File System
*
* File: exents.h
*
* Description: 
*   Header file for extents that will handel non-contiguous allocation 
*
**************************************************************/

#ifndef _EXTENTS_H
#define _EXTENTS_H

#include "directory.h"

#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif

/**
 * Gets lba position for block 'n' of the file
 */
int GetLBAfromFileBlockN(directoryEntry *buffer, int n);

/**
 * Allocates free space using extents
 * Returns location of where first block is stored
 * Returns -1 if allocation fails
 */
int AllocateBlocksInExtents(directoryEntry *buffer, int n);

#endif
