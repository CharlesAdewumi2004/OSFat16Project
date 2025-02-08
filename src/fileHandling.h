#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "readFat16File.h"

typedef struct{ 
    BootSector *bs;
    RootDir *rDir;
    uint16_t *FAT;
    char fileName[50];
    
}File;

File *openFile(BootSector *bs, RootDir *rDir, uint16_t *FAT);
void closeFile(File *file);
void selectFile(File *file);
void seekFile(off_t offset, File *file, int whence);
