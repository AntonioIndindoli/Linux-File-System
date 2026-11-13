/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "b_io.h"
#include "directory.h"
#include "fileInfo.h"
#include "volumeControlBlock.h"
#include "fsLow.h"
#include "mfs.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
{
	/** TODO add al the information you need in the file control block **/
	directoryEntry *fi; // holds the low level system file info
	int flags;			// holds the creation flags
	char *buf;			// holds the open file buffer
	int index;			// holds the current position in the buffer
	int buflen;			// holds how many valid bytes are in the buffer
	int fileIndex;		// holds the current position in the file
	int currentBlock;	//hold current block number
	int numBlocks;		//hold how many blocks file occupies
} b_fcb;

b_fcb fcbArray[MAXFCBS];

int startup = 0; //Indicates that this has not been initialized

//Method to initialize our file system
void b_init()
{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
	{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
	}

	startup = 1;
}

//Method to get a free FCB element
b_io_fd b_getFCB()
{
	for (int i = 0; i < MAXFCBS; i++)
	{
		if (fcbArray[i].buf == NULL)
		{
			return i; //Not thread safe (But do not worry about it for this assignment)
		}
	}
	return (-1); //all in use
}

// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open(char *filename, int flags)
{
	b_io_fd fd;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//

	if (startup == 0)
		b_init(); //Initialize our system

	fd = b_getFCB(); // get our own file descriptor
					 // check for error - all used FCB's
	if (fd < 0)
	{
		fprintf(stderr, "Reached the max number of files open\n");
		return -1;
	}

	printf("filename %s\n", filename);
	ParseDir *parse = parsePath(filename);
	if (parse->exists == 2)
	{
		fprintf(stderr, "Invalid path\n");
		return -1;
	}

	int isWrite = 0;

	// set read only and write only flags
	if (flags & O_RDONLY == O_RDONLY)
	{
		// can not call  : b_write on the file
		// can ONLY call : b_read on file
		fcbArray[fd].flags = fcbArray[fd].flags | O_RDONLY;
	}
	else if (flags & O_WRONLY == O_WRONLY)
	{
		// can not call  : b_read on the file
		// can ONLY call : b_write on the file
		fcbArray[fd].flags = fcbArray[fd].flags | O_WRONLY;
		isWrite = 1;
	}
	else if (flags & O_RDWR == O_RDWR)
	{
		// can not call  : b_read on the file
		// can ONLY call : b_write on the file
		fcbArray[fd].flags = fcbArray[fd].flags | O_RDWR;
		isWrite = 1;
	}

	int isTruncate = 0;
	if ((flags & O_TRUNC == O_TRUNC) && isWrite == 1)
	{
		if (parse->exists == 0 && parse->dir[parse->location].isDirectory == 0)
		{
			isTruncate = 1;
			// should we dealloc and update the freespace map
			// since filesize is truncated to 0?
		}
	}

	// create a file
	// printf( "open\n" );
	// printf( "%d\n",parse->exists );
	if (flags & O_CREAT == O_CREAT && parse->exists == 1)
	{
		// printf( "make\n" );
		fcbArray[fd].fi = fs_makeFile(parse->dir, filename);

		if (isTruncate == 1)
		{
			fcbArray[fd].fi->fileSize = 0;
		}

		int numBlocks = (parse->dir[0].fileSize + (vcb->sizeBlocks - 1)) / vcb->sizeBlocks;
		LBAwrite(parse->dir, numBlocks, parse->dir[0].location);
	}
	else
	{
		// open file, error if it doesnt exist
		if (parse->exists == 1)
		{
			fprintf(stderr, "File doesnt exist\n");

			return -1;
		}

		fcbArray[fd].fi = loadDirectory(parse->dir[parse->location]);
		if (isTruncate == 1)
		{
			fcbArray[fd].fi->fileSize = 0;
			parse->dir[parse->location].fileSize = 0;
		}

		int numBlocks = (parse->dir[0].fileSize + (vcb->sizeBlocks - 1)) / vcb->sizeBlocks;
		LBAwrite(parse->dir, numBlocks, parse->dir[0].location);
	}

	// TODO consider malloc'ing in read()
	fcbArray[fd].buf = malloc(vcb->sizeBlocks);
	if (fcbArray[fd].buf == NULL)
	{
		fprintf(stderr, "Memory Allocation Error : b_open\n");

		exit(EXIT_FAILURE);
	}

	fcbArray[fd].buflen = vcb->sizeBlocks;
	fcbArray[fd].index = 0;
	fcbArray[fd].fileIndex = 0;

	return (fd); // all set
}

// Interface to seek function
int b_seek(b_io_fd fd, off_t offset, int whence)
{
	if (startup == 0)
		b_init(); //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); //invalid file descriptor
	}

	if (whence < 0 || whence > 2)
	{
		return -1;
	}

	if (whence == SEEK_SET)
	{
		// file offset is set to the offset bytes
		fcbArray[fd].fileIndex = offset;
	}
	else if (whence == SEEK_CUR)
	{
		// the file offset is set to its current location plus offset bytes
		fcbArray[fd].fileIndex += offset;
	}
	else if (whence == SEEK_END)
	{
		// the file offset is set to the size of the file plus offset bytes
		fcbArray[fd].fileIndex = fcbArray[fd].fi->fileSize + offset;
	}

	return fcbArray[fd].fileIndex; //Change this
}

// Interface to write function
int b_write(b_io_fd fd, char *buffer, int count)
{
	/*
	if (startup == 0)
		b_init(); //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); //invalid file descriptor
	}

	// and check that the specified FCB is actually in use
	if (fcbArray[fd].fi == NULL) //File not open for this descriptor
	{
		return -1;
	}

	int totalBytes = 0;
	int bytesToRead = count; // tracks the # of bytes we need to read
	int remainingBytesInMyBuffer = fcbArray[fd].buflen - fcbArray[fd].index;
	int writeLocation = AllocateBlocksInExtents(fcbArray[fd].fi, count/BLOCK_SIZE);

	/*
	// if the amount of bytes to read will overflow our buffer, then fill the buffer completely and
	// copy those bytes we used to fill it to the caller buffer. Then move to the next data block
	if ((remainingBytesInMyBuffer + bytesToRead) > B_CHUNK_SIZE)
	{
		int bytesToReadTemp = B_CHUNK_SIZE - remainingBytesInMyBuffer;

		LBAread(fcbArray[fd].buf, 1, writeLocation);
		memcpy(buffer, fcbArray[fd].buf + remainingBytesInMyBuffer, bytesToReadTemp);

		remainingBytesInMyBuffer = 0;
		writeLocation++;
		bytesToRead -= bytesToReadTemp;
		totalBytes += bytesToReadTemp;
	}
	

	
	// if the amount of bytes to read is greater than a chunk, then copy the next block(s) directly
	// into the caller's buffer
	if (bytesToRead > B_CHUNK_SIZE)
	{
		for (int i = 0; i < (bytesToRead / B_CHUNK_SIZE); i++)
		{
			LBAwrite(buffer + totalBytes, 1, writeLocation);
			remainingBytesInMyBuffer = 0;
			writeLocation++;
			totalBytes += B_CHUNK_SIZE;
			bytesToRead -= B_CHUNK_SIZE;
		}
	}

	// read any remaining bytes into our buffer and then copy it to the caller's buffer
	LBAread(fcbArray[fd].buf, 1, writeLocation);
	memcpy(buffer + totalBytes, fcbArray[fd].buf + remainingBytesInMyBuffer, bytesToRead);

	totalBytes += bytesToRead;
	remainingBytesInMyBuffer += bytesToRead;
	//fcbArray[fd].bytesRead += totalBytes;

	*/
	return 0;
}

// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+

int b_read(b_io_fd fd, char *buffer, int count)
{
	int bytesRead;
	int bytesReturned;
	int part1, part2, part3;
	int numberOfBlocksToCopy;
	int remainingBytesInMyBuffer;
	if (startup == 0)
		b_init(); //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); //invalid file descriptor
	}

	remainingBytesInMyBuffer = fcbArray[fd].buflen - fcbArray[fd].index;

	//limit count to file size
	int amountAlreadyDelivered = (fcbArray[fd].currentBlock * B_CHUNK_SIZE) - remainingBytesInMyBuffer;
	if ((count + amountAlreadyDelivered) > fcbArray[fd].fi->fileSize)
	{
		count = fcbArray[fd].fi->fileSize - amountAlreadyDelivered;
		if (count < 0)
		{
			printf("fail: read more than fileSize\n");
			return -1; //	error
		}
	}
	//check if count less than byte in buffer
	if (count <= remainingBytesInMyBuffer)
	{
		part1 = count;
		part2 = 0;
		part3 = 0;
	}
	else
	{
		part1 = remainingBytesInMyBuffer;
		part3 = count - remainingBytesInMyBuffer;
		numberOfBlocksToCopy = part3 / B_CHUNK_SIZE;
		part2 = numberOfBlocksToCopy * B_CHUNK_SIZE;
		part3 = part3 - part2;
	}

	if (part1 > 0)
	{
		memcpy(buffer, fcbArray[fd].buf + fcbArray[fd].index, part1);
		fcbArray[fd].index = fcbArray[fd].index + part1;
	}

	if (part2 > 0)
	{
		bytesRead = LBAread(buffer + part1, numberOfBlocksToCopy, fcbArray[fd].currentBlock + 1);
		fcbArray[fd].currentBlock += numberOfBlocksToCopy;
		part2 = bytesRead;
	}

	if (part3 > 0)
	{
		bytesRead = LBAread(fcbArray[fd].buf, 1, fcbArray[fd].currentBlock + 1);
		fcbArray[fd].currentBlock += 1;
		fcbArray[fd].index = 0;
		fcbArray[fd].buflen = bytesRead;

		if (bytesRead < part3)
		{
			part3 = bytesRead;
		}

		if (part3 > 0)
		{
			memcpy(buffer + part1 + part2, fcbArray[fd].buf + fcbArray[fd].index, part3);
			fcbArray[fd].index = fcbArray[fd].index + part3;
		}
	}
	bytesReturned = part1 + part2 + part3;

	return bytesReturned; //Change this
}

// Interface to Close the file
int b_close(b_io_fd fd)
{
	free(fcbArray[fd].buf);
	fcbArray[fd].buf = NULL;
}
