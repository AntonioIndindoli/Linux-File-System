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
    int exists;          // 1 if last element exits, 0 if it doesnt
    int location;        // index location of the last element Directory if it exists

} ParseDir;

struct ParseDir *parsePath(char *pathname)
{
    ParseDir *pathInfo;
    ParseDir *previousDir;
    int isRoot = 0;
    int dirExists = 0;
    char *dirName;
    char *lastdirName;

    if (pathname[0] == '/')
    {
        isRoot = 1;
        LBAread(cwd, 5, 6); //set current dir to root
        dirName = strtok(pathname, '/');
        dirName = strtok(NULL, pathname);
    }
    else
    {
        dirName = strtok(pathname, '/');
    }

    while (dirName != NULL)
    {
        previousDir = cwd;
        lastdirName = dirName;
        dirName = strtok(NULL, pathname);

        if (strcmp(cwd[1].name, dirname) != 0 && dirName != NULL)
        {
            printf("Invalid path");
            return NULL;
        } 
        else if (strcmp(cwd[1].name, lastdirName) == 0)
        {
            dirExists = 1;
            LBAread(cwd, 5, cwd[1].location);
        }
    }

    pathInfo->location = cwd.location;
    pathInfo->dir = previousDir;
    pathInfo->exists = dirExists;

    return pathInfo;
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