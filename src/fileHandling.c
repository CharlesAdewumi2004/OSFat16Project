#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include "readFat16File.h"

File *openFile(BootSector *bs, RootDir *rDir, uint16_t *FAT) {
    File *file = malloc(sizeof(File));
    if (file == NULL) {
        perror("Failed to allocate memory for file");
        return NULL;
    }
    
    memset(file->fullFileName, 0, sizeof(file->fullFileName));
    file->clusterChain = NULL;

    getFileName(file->fullFileName);
    
    convertToFat83(file->fullFileName, file->partialFileName);

    RootDir *fileDirEntry = findFileClusters(file->partialFileName, rDir, bs->BPB_RootEntCnt);
    if (fileDirEntry == NULL) {
        printf("Error: File not found in FAT16 root directory.\n");
        free(file);
        return NULL;
    }

    addElementToList(&(file->clusterChain), fileDirEntry->DIR_FstClusLO);
    
    addClustersToLinkedList(fileDirEntry->DIR_FstClusLO, FAT, file->clusterChain);

    return file;
}

void convertToFat83(const char *filename, char *fatName) {
    memset(fatName, ' ', 11); 
    fatName[11] = '\0'; 

    int nameLen = 0, extLen = 0;
    const char *dot = strrchr(filename, '.');  

    if (dot) {
        nameLen = dot - filename; 
        extLen = strlen(dot + 1); 
    } else {
        nameLen = strlen(filename);
    }

    for (int i = 0; i < 8 && i < nameLen; i++) {
        fatName[i] = toupper(filename[i]); 
    }

    if (dot && extLen > 0) {
        for (int i = 0; i < 3 && dot[i + 1] != '\0'; i++) {
            fatName[8 + i] = toupper(dot[i + 1]);
        }
    }
}

void getFileName(char inputFileName[255]) {
    printf("Enter filename:\t");
    if (fgets(inputFileName, 255, stdin) == NULL) {
        printf("Error reading input.\n");
        return;
    }
    inputFileName[strcspn(inputFileName, "\n")] = 0;  
}

off_t seekFileCluster(BootSector *bs, File *file, int fd) {
    off_t startPoint = bs->BPB_RsvdSecCnt * bs->BPB_BytsPerSec;//bootsector

    startPoint += bs->BPB_FATSz16 * bs->BPB_NumFATs * bs->BPB_BytsPerSec;//fat

    off_t rootDirSize = ((bs->BPB_RootEntCnt * 32) + (bs->BPB_BytsPerSec - 1)) / bs->BPB_BytsPerSec;//root dir
    startPoint += rootDirSize * bs->BPB_BytsPerSec;

    LinkedList *currentNode = file->clusterChain;
    uint16_t firstCluster = currentNode->clusterNum;
    if (firstCluster < 2) {  
        return -1; 
    }
    int offset = startPoint + (firstCluster - 2) * (bs->BPB_SecPerClus * bs->BPB_BytsPerSec);
    
    lseek(fd, offset, SEEK_SET);

    return startPoint;
}

void readFile(File *file,int fd, BootSector *bs){
    file->fileContents = malloc(bs->BPB_BytsPerSec * bs->BPB_SecPerClus + 1);
    if (file->fileContents == NULL) {
        perror("error allocating memeory");
        return;
    }
    int bytesRead = read(fd, file->fileContents, bs->BPB_BytsPerSec * bs->BPB_SecPerClus - 1); 
    if (bytesRead > 0) {
        file->fileContents[bytesRead] = '\0';
        printf("%s", file->fileContents);
    }else{
        perror("error reading file");
    }   
}


void closeFile(File *file) {
    if (file == NULL) return;
    free(file);
}
