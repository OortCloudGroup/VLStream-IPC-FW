#ifndef _COMMON_H_
#define _COMMON_H_

// kernal data struct

#include <stdio.h>  
#include <string.h> 
#include <stdlib.h>  
#include <dirent.h>  
#include <sys/stat.h>  
#include <unistd.h>  
#include <sys/types.h> 
#include <ctype.h>
//#include "index.h"
//#include "search.h"

#define MAXCHARDEFINED 200
#define WORDCHARS "abcdefghijklmnopqrstuvwxyz懒旅呐魄壬仕掏蜗醒矣哉重寠仝圮轃捺徕沅彐玷殛腱眍镳耱篝貊鴾氝浮?0123456789"




typedef struct fileinfo {
	char *filename;
	char *path;
    int id;
	struct fileinfo *next;
}FILEINFO;

typedef struct locationinfo{
    int  frequency;
    int id;
    struct locationinfo *next;
}LOCATIONINFO;

typedef struct wordinfo{
    char *word;
    struct locationinfo *locate;
    struct wordinfo *next;
}WORDINFO;



// for whole application
typedef struct __tag_index_context{
    int count;
    FILEINFO * filelist;
    WORDINFO * wordlist;
    
}INDEX_CONTEXT;

extern INDEX_CONTEXT index_context;



void IndexFiles(char * path);
char * Process_File_Type(char * file);

#endif