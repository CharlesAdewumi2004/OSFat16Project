#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "readFat16File.h"

// Function to open a file in FAT16
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

// Convert a normal filename to FAT16 8.3 format
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

// Get file name input from user safely
void getFileName(char inputFileName[255]) {
    printf("Enter filename:\t");
    if (fgets(inputFileName, 255, stdin) == NULL) {
        printf("Error reading input.\n");
        return;
    }
    inputFileName[strcspn(inputFileName, "\n")] = 0;  
}

// Close the file and free memory
void closeFile(File *file) {
    if (file == NULL) return;
    
    

    free(file);
}
