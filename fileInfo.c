/**************************************************************
* Class:  CSC-415-02 Fall 2022
* Names: Evan Giampaoli, Nathan Rennacker, Hann Zhao, Antonio Indindoli 
* Student IDs: 922361476, 921348958, 917565433, 920356733
* GitHub Name: hazhao33
* Group Name: EHNA
* Project: Basic File System
*
* File: fileInfo.c
*
* Description: Holds function and struct for returning the necessary
* information of a file 
*
*
**************************************************************/

#include "directory.h"
#include "extents.h"
#include "mfs.h"
#include "fileInfo.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

fileInfo* GetFileInfo( const char* filename, int flag ) {
    ParseDir* parse = parsePath( filename );
    
    fileInfo* fi;

    if (parse->dir[parse->location].isDirectory == 1) {
        fprintf( stderr, "Error, can't open directory as a file\n" );
        return NULL;
    } 
    else if (parse->exists == 2) {
        fprintf( stderr, "Invalid path\n" );
        return NULL;
    }
    else if (parse->exists == 1) {
        // file does not exist, so create it
    }
    else {
        // file exists, open it
        fi = malloc( sizeof( fileInfo ) );
        if (fi == NULL) {
            fprintf( stderr, "Malloc Error : getfileinfo\n" );
            exit( EXIT_FAILURE );
        }

        // fi->de = loadDirectory( parse->dir[parse->location] );
        strcpy( fi->fileName, parse->dir[parse->location].name );
        
        // use locations until switch to extents
        fi->location = parse->location; 

        return fi;
    }

    return NULL;
}