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

#include "mfs.h"
#include "directory.h"

typedef struct ParseDir
{
	directoryEntry *dir; // (n - 1) element Directory in memory
	int exists;			 // 1 if last element exits, 0 if it doesnt
	int location;		 // index location of the last element Directory if it exists

} ParseDir;

struct ParseDir parsePath(const char *pathname)
{
    ParseDir *pathInfo;
    int isRoot = 0;
    char *dirName;

    if (pathname[0] == '/'){
        isRoot  = 1;
    }

    int doOnce = 0;
    while (dirName != NULL)
    {
        if(doOnce = 0){
            dirName = strtok(pathname, '/');
            if(isRoot == 1){
            dirName = strtok(NULL, pathname);
            //if(not in working directory)
            //then return error
            }
        }

        //if(not in loaded directory)
        //then return error
        //else load directory

        dirName = strtok(NULL, pathname);
        doOnce = 1;
    }

    return *pathInfo;
}

fdDir *fs_opendir(const char *pathname)
{

    return NULL;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    return NULL;
}

int fs_closedir(fdDir *dirp)
{

    return 0;
}