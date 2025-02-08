#include "fileHandling.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

File *openFile(BootSector *bs, RootDir *rDir, uint16_t *FAT){
    File *file = malloc(sizeof(File));
    file->bs = bs;
    file->rDir = rDir;
    file->FAT = FAT;

    return file;
}

void selectFile(File *file){
    char fileName[50];
    printf("enter file name: \t");
    scanf("%s", fileName);
    strcpy(file->fileName, fileName);
}

void seekFile(off_t offset, File *file, int whence){
    offset = whence + (file->bs->BPB_BytsPerSec * file->bs->BPB_RsvdSecCnt) + (file->bs->BPB_FATSz16 * 
                        file->bs->BPB_NumFATs * file->bs->BPB_BytsPerSec) + (file->bs->BPB_RootEntCnt 
                        * sizeof(*file->rDir));
    //todo
}

void closeFile(File *file){
    free(file);
}