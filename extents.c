/**************************************************************
* Class:  CSC-415-02 Fall 2022
* Names: Evan Giampaoli, Nathan Rennacker, Hann Zhao, Antonio Indindoli 
* Student IDs: 922361476, 921348958, 917565433, 920356733
* GitHub Name: hazhao33
* Group Name: EHNA
* Project: Basic File System
*
* File: extents.c
*
* Description: Extents functions
*
* Includes the functions to find and allocate blocks in the extents system.
*
**************************************************************/
#include "extents.h"
#include "directory.h"
#include "bitMap.h"
#include "fsLow.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int extentCount = (BLOCK_SIZE / (sizeof(int) * 2));    //how many extent entries can fit in one block
int extentPointerCount = (BLOCK_SIZE / (sizeof(int))); //how many pointers can fit in one block

/**
 * Adds entry to extents table
 * Takes in block position of table and how many blocks to allocate
 * Returns 0 if successful
 * Returns -1 if allocation fails
 */
int AddEntryToExtentsTable(int blockPos, int blocksToAllocate);

/**
 * Finds a free block to use as an extents table
 * Takes in first values to assign and blocks to allocate
 * Passing in -1 for firstBlockEntry and firstCountEntry skips first entry assignment
 * Returns block position of table if successful
 * Returns -1 if allocation fails
 */
int CreateExtentsTable(int firstBlockEntry, int firstCountEntry, int blocksToAllocate);

/**
 * Finds a free block to use as a table of pointers to extent tables
 * Takes in first value to assign and blocks to allocate
 * Passing in -1 for firstEntry skips first entry assignment
 * Returns block position of table if successful
 * Returns -1 if allocation fails
 */
int CreatePointerTable(int firstEntry, int blocksToAllocate);

int GetLBAfromFileBlockN(directoryEntry *buffer, int n)
{
    int searchIndirect = -1;
    int i = 0;
    while (buffer->ext[i].count <= n)
    {
        if (buffer->ext[i].count < 0)
        {
            searchIndirect = 0;
            break;
        }
        n -= buffer->ext[i].count;
        i++;
    }

    if (searchIndirect == -1)
        return buffer->ext[i].blockNumber + n;

    if (buffer->ext[2].count == -3)
    {
        int *indirectExt;
        LBAread(indirectExt, 1, buffer->ext[2].blockNumber);
        int i;
        for (i = 0; i < extentPointerCount; i++)
        {
            int *innerIndirectExt;
            LBAread(innerIndirectExt, 1, indirectExt[i]);
            int j;
            for (j = 0; j < extentPointerCount; j++)
            {
                Extent *nextInnerExt;
                LBAread(nextInnerExt, 1, innerIndirectExt[j]);
                int h;
                for (h = 0; h < extentCount; h++)
                {
                    n -= nextInnerExt[h].count;
                    if (nextInnerExt[i].count > n)
                        return nextInnerExt[i].blockNumber + n;
                }
            }
        }
        fprintf(stderr, "Could not find LBA\n");
        return -1;
    }
    else if (buffer->ext[2].count == -2)
    {
        int *indirectExt;
        LBAread(indirectExt, 1, buffer->ext[2].blockNumber);
        int j;
        for (j = 0; j < extentPointerCount; j++)
        {
            Extent *nextInnerExt;
            LBAread(nextInnerExt, 1, indirectExt[j]);
            int h;
            for (h = 0; h < extentCount; h++)
            {
                n -= nextInnerExt[h].count;
                if (nextInnerExt[i].count > n)
                    return nextInnerExt[i].blockNumber + n;
            }
        }
        fprintf(stderr, "Could not find LBA\n");
        return -1;
    }
    else
    {
        Extent *indirectExt;
        LBAread(indirectExt, 1, buffer->ext[2].blockNumber);
        int h;
        for (h = 0; h < extentCount; h++)
        {
            n -= indirectExt[h].count;
            if (indirectExt[i].count > n)
                return indirectExt[i].blockNumber + n;
        }
        fprintf(stderr, "Could not find LBA\n");
        return -1;
    }
}

int AllocateBlocksInExtents(directoryEntry *buffer, int n)
{
    int blocktoFind = buffer->fileSize/BLOCK_SIZE+1;
    if (buffer->ext[2].count == -3)
    {
        //adding entry to third indirect extents table
        int *indirectExt;
        LBAread(indirectExt, 1, buffer->ext[2].blockNumber);
        int i;
        for (i = 0; i < extentPointerCount; i++)
        {
            if (indirectExt[i] == -1)
            {
                int *innerIndirectExt;
                LBAread(innerIndirectExt, 1, indirectExt[i - 1]);
                int j;
                for (j = 0; j < extentPointerCount; j++)
                {
                    if (innerIndirectExt[j] == -1)
                    {
                        int result = AddEntryToExtentsTable(innerIndirectExt[j - 1], n);
                        if (result == 0)
                            return GetLBAfromFileBlockN(buffer, blocktoFind);

                        innerIndirectExt[j] = CreateExtentsTable(-1, -1, n);

                        LBAwrite(innerIndirectExt, 1, indirectExt[i - 1]);
                        return GetLBAfromFileBlockN(buffer, blocktoFind);
                    }
                }

                indirectExt[i] = CreatePointerTable(-1, n);
                if (indirectExt[i] == -1)
                {
                    fprintf(stderr, "Failed to allocate space on disk\n");
                    return -1;
                }
                LBAwrite(indirectExt, 1, buffer->ext[2].blockNumber);
                return GetLBAfromFileBlockN(buffer, blocktoFind);
            }
        }
        fprintf(stderr, "Maximum file size reached\n");
        return -1;
    }
    else if (buffer->ext[2].count == -2)
    {
        //adding entry to second indirect extents table
        int *indirectExt;
        LBAread(indirectExt, 1, buffer->ext[2].blockNumber);
        int i;
        for (i = 0; i < extentPointerCount; i++)
        {
            if (indirectExt[i] == -1)
            {
                int result = AddEntryToExtentsTable(indirectExt[i - 1], n);
                if (result == 0)
                    return GetLBAfromFileBlockN(buffer, blocktoFind);

                indirectExt[i] = CreateExtentsTable(-1, -1, n);

                LBAwrite(indirectExt, 1, buffer->ext[2].blockNumber);
                return GetLBAfromFileBlockN(buffer, blocktoFind);
            }
        }
        //creating third indirect extent if there is so space
        int indirectExtPos = allocateBlocks(1);
        if (indirectExtPos == -1)
        {
            fprintf(stderr, "Failed to allocate space on disk\n");
            return -1;
        }

        //int thirdIndirectExt[extentPointerCount];

        int *thirdIndirectExt = (int *)malloc(extentPointerCount * sizeof(int));
        if (thirdIndirectExt == NULL)
        {
            fprintf(stderr, "Malloc failed\n");
            return -1;
        }

        for (int i = 0; i < extentPointerCount; i++)
        {
            //thirdIndirectExt[i] = malloc(sizeof(int));
            thirdIndirectExt[i] = -1;
        }
        thirdIndirectExt[0] = buffer->ext[2].blockNumber;

        buffer->ext[2].blockNumber = indirectExtPos;
        buffer->ext[2].count = -3;

        thirdIndirectExt[1] = CreatePointerTable(-1, n);
        if (thirdIndirectExt[1] == -1)
        {
            fprintf(stderr, "Failed to allocate space on disk\n");
            return -1;
        }
        LBAwrite(thirdIndirectExt, 1, indirectExtPos);
        return GetLBAfromFileBlockN(buffer, blocktoFind);
    }
    else if (buffer->ext[2].count == -1)
    {
        //adding entry to indirect extents table
        int result = AddEntryToExtentsTable(buffer->ext[2].blockNumber, n);
        if (result == 0)
            return GetLBAfromFileBlockN(buffer, blocktoFind);

        //creating second indirect extent if there is no space
        result = CreatePointerTable(buffer->ext[2].blockNumber, n);
        if (result == -1)
        {
            fprintf(stderr, "Failed to allocate space on disk\n");
            return -1;
        }
        buffer->ext[2].blockNumber = result;
        buffer->ext[2].count = -2;
        return GetLBAfromFileBlockN(buffer, blocktoFind);
    }
    else if (buffer->ext[2].count > 0)
    {
        //creating first indirect extent if there is no space
        int result = CreateExtentsTable(buffer->ext[2].blockNumber, buffer->ext[2].count, n);
        if (result == -1)
        {
            fprintf(stderr, "Failed to allocate space on disk\n");
            return -1;
        }
        buffer->ext[2].blockNumber = result;
        buffer->ext[2].count = -1;
        return GetLBAfromFileBlockN(buffer, blocktoFind);
    }
    else
    {
        //if there are no indirect exents
        int blockPos = allocateBlocks(n);
        if (blockPos == -1)
        {
            fprintf(stderr, "Failed to allocate space on disk\n");
            return -1;
        }
        if (buffer->ext[0].count == 0)
        {
            buffer->ext[0].blockNumber = blockPos;
            buffer->ext[0].count = n;
        }
        else if (buffer->ext[1].count == 0)
        {
            buffer->ext[1].blockNumber = blockPos;
            buffer->ext[1].count = n;
        }
        else
        {
            buffer->ext[2].blockNumber = blockPos;
            buffer->ext[2].count = n;
        }
        return GetLBAfromFileBlockN(buffer, blocktoFind);
    }
}

int AddEntryToExtentsTable(int blockPos, int blocksToAllocate)
{
    Extent *indirectExt;
    LBAread(indirectExt, 1, blockPos);
    int i;
    for (i = 0; i < extentCount; i++)
    {
        if (indirectExt[i].blockNumber == -1)
        {
            //add entry to indirect extent
            blockPos = allocateBlocks(blocksToAllocate);
            if (blockPos == -1)
            {
                fprintf(stderr, "Failed to allocate space on disk\n");
                return -1;
            }
            indirectExt[i].blockNumber = blockPos;
            indirectExt[i].count = blocksToAllocate;

            LBAwrite(indirectExt, 1, blockPos);
            return 0;
        }
    }
    return -1;
}

int CreateExtentsTable(int firstBlockEntry, int firstCountEntry, int blocksToAllocate)
{
    int indirectExtPos = allocateBlocks(1);
    if (indirectExtPos == -1)
    {
        fprintf(stderr, "Failed to allocate space on disk\n");
        return -1;
    }

    //Extent indirectExt[extentCount];

    Extent *indirectExt = (Extent *)malloc(extentCount * sizeof(Extent));
    if (indirectExt == NULL)
    {
        fprintf(stderr, "Malloc failed\n");
        return -1;
    }

    for (int i = 0; i < extentCount; i++)
    {
        /*
        indirectExt[i].blockNumber = malloc(sizeof(int));
        indirectExt[i].count = malloc(sizeof(int));
        if (indirectExt[i].blockNumber == NULL || indirectExt[i].count == NULL)
        {
            fprintf( stderr, "Malloc failed\n");
            return -1;
        }
        */
        indirectExt[i].blockNumber = -1;
    }

    int allocationPlacement = 0;

    if (firstBlockEntry != -1 && firstCountEntry != -1)
    {
        indirectExt[0].blockNumber = firstBlockEntry;
        indirectExt[0].count = firstCountEntry;
        allocationPlacement = 1;
    }
    int blockPos = allocateBlocks(blocksToAllocate);
    if (blockPos == -1)
    {
        fprintf(stderr, "Failed to allocate space on disk\n");
        return -1;
    }

    indirectExt[allocationPlacement].blockNumber = blockPos;
    indirectExt[allocationPlacement].count = blocksToAllocate;
    LBAwrite(indirectExt, 1, indirectExtPos);
    return indirectExtPos;
}

int CreatePointerTable(int firstEntry, int blocksToAllocate)
{
    int indirectExtPos = allocateBlocks(1);
    if (indirectExtPos == -1)
    {
        fprintf(stderr, "Failed to allocate space on disk\n");
        return -1;
    }

    int *pointerIndirectExt = (int *)malloc(extentPointerCount * sizeof(int));
    if (pointerIndirectExt == NULL)
    {
        fprintf(stderr, "Malloc failed\n");
        return -1;
    }

    //int pointerIndirectExt[extentPointerCount];
    for (int i = 0; i < extentPointerCount; i++)
    {
        /*
        pointerIndirectExt[i] = malloc(sizeof(int));
        if (pointerIndirectExt[i] == NULL)
        {
            fprintf( stderr, "Malloc failed\n");
            return -1;
        }
        */
        pointerIndirectExt[i] = -1;
    }

    int firstEntryPos = 0;
    if (firstEntry != -1)
    {
        pointerIndirectExt[0] = firstEntry;
        firstEntryPos = 1;
    }

    pointerIndirectExt[firstEntryPos] = CreateExtentsTable(-1, -1, blocksToAllocate);
    if (pointerIndirectExt[firstEntryPos] == -1)
    {
        fprintf(stderr, "Failed to allocate space on disk\n");
        return -1;
    }
    LBAwrite(pointerIndirectExt, 1, indirectExtPos);
    return indirectExtPos;
}
