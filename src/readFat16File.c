#include "readFat16File.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <endian.h>

#define BOOTSECTORSIZE 512
#define FATENTRYSIZE 2

// Print Boot Sector Information
void printBootSector(BootSector *bs){    
    printf("OEM Name: %.8s\n", bs->BS_OEMName);
    printf("Bytes per Sector: %d\n", bs->BPB_BytsPerSec);
    printf("Sectors per Cluster: %d\n", bs->BPB_SecPerClus);
    printf("Reserved Sectors: %d\n", bs->BPB_RsvdSecCnt);
    printf("Number of FATs: %d\n", bs->BPB_NumFATs);
    printf("Root Entry Count: %d\n", bs->BPB_RootEntCnt);
    printf("Total Sectors (16-bit): %d\n", bs->BPB_TotSec16);
    printf("Media Descriptor: 0x%02X\n", bs->BPB_Media);
    printf("Sectors per FAT: %d\n", bs->BPB_FATSz16);
    printf("Volume Label: %.11s\n", bs->BS_VolLab);
    printf("File System Type: %.8s\n", bs->BS_FilSysType);
}

// Read Boot Sector
BootSector *readFat16ImageBootSector(int fd) {
    BootSector *bs = malloc(sizeof(BootSector));
    if (!bs) {
        perror("Error allocating memory for BootSector");
        return NULL;
    }

    lseek(fd, 0, SEEK_SET);
    if (read(fd, bs, sizeof(*bs)) != sizeof(*bs)) {
        perror("Error reading Boot Sector");
        free(bs);
        return NULL;
    }
    return bs;
}

// Read FAT Table
uint16_t *readFat16Fat(int fd, BootSector *bs) {
    uint16_t fatSize = bs->BPB_FATSz16 * bs->BPB_BytsPerSec;
    uint16_t *FAT = malloc(fatSize);
    if (!FAT) {
        perror("Failed to allocate memory for FAT");
        return NULL;
    }

    lseek(fd, bs->BPB_RsvdSecCnt * bs->BPB_BytsPerSec, SEEK_SET);
    if (read(fd, FAT, fatSize) < 0) {
        perror("Error reading FAT table");
        free(FAT);
        return NULL;
    }
    return FAT;
}

// Read a Cluster Chain
void readCluster(int startCluster, uint16_t *FAT) {
    uint16_t cluster = startCluster;
    printf("Cluster Chain: ");
    
    while (cluster < 0xFFF8) { 
        printf("%u -> ", cluster);
        cluster = FAT[cluster];  
    }

    printf("EOF\n");
}

// Add Clusters to Linked List
void addClustersToLinkedList(int startCluster, uint16_t *FAT, LinkedList *root) {
    uint16_t cluster = startCluster;
    
    while (cluster < 0xFFF8) { 
        addElementToList(&root, cluster);
        cluster = FAT[cluster];  
    }
}

// Print FAT Table Entries
void printNAmountOfFatSection(int n, uint16_t *FAT) {
    printf("First %d Entries\n", n);
    for (int i = 2; i < n+2; i++) {
        printf("FAT[%d] → 0x%04X\n", i, FAT[i]);
    }
}

// Read Root Directory
RootDir *readRootDir(int fd, BootSector *bs) {
    uint32_t rootDirStart = (bs->BPB_RsvdSecCnt + (bs->BPB_NumFATs * bs->BPB_FATSz16)) * bs->BPB_BytsPerSec;
    uint32_t rootDirSize = bs->BPB_RootEntCnt * sizeof(RootDir);

    RootDir *rDir = malloc(rootDirSize);
    if (!rDir) {
        perror("Error allocating memory for Root Directory");
        return NULL;
    }

    lseek(fd, rootDirStart, SEEK_SET);
    if (read(fd, rDir, rootDirSize) != rootDirSize) {
        perror("Error reading Root Directory");
        free(rDir);
        return NULL;
    }
    return rDir;
}

// Find a File in Root Directory
RootDir *findFileClusters(const char *fileName, RootDir *rDir, int totalEntries) {
    for (int i = 0; i < totalEntries; i++) {
        if (rDir[i].DIR_Name[0] == 0x00) { 
            break;
        }
        if (rDir[i].DIR_Name[0] == 0xE5) {
            continue;
        }

        char fatFileName[12];
        memset(fatFileName, 0, sizeof(fatFileName));
        memcpy(fatFileName, rDir[i].DIR_Name, 11);
        fatFileName[11] = '\0';
        if (strncmp(fatFileName, fileName, 8) == 0) {   
            return &rDir[i]; 
        }
    }
    return NULL; 
}

void extractLFNChars(char *buffer, uint8_t *src, int count) {
    int index = 0;
    for (int i = 0; i < count; i += 2) {
        if (src[i] == 0xFF || src[i] == 0x00) break;  
        buffer[index++] = src[i];
    }
    buffer[index] = '\0';
}

off_t getClusterOffset(BootSector *bs, uint16_t cluster) {
    off_t startOffset = (bs->BPB_RsvdSecCnt + bs->BPB_NumFATs * bs->BPB_FATSz16) * bs->BPB_BytsPerSec;

    startOffset += (bs->BPB_RootEntCnt * 32 + (bs->BPB_BytsPerSec - 1)) / bs->BPB_BytsPerSec * bs->BPB_BytsPerSec;

    return startOffset + (cluster - 2) * (bs->BPB_SecPerClus * bs->BPB_BytsPerSec);
}

RootDir *readDirectoryFromCluster(int fd, BootSector *bs, uint16_t startCluster) {

    off_t dirStart = getClusterOffset(bs, startCluster);
    int numEntries = (bs->BPB_SecPerClus * bs->BPB_BytsPerSec) / sizeof(RootDir);

    RootDir *directory = malloc(numEntries * sizeof(RootDir));
    if (!directory) {
        perror("Error allocating memory for directory");
        return NULL;
    }

    lseek(fd, dirStart, SEEK_SET);
    if (read(fd, directory, numEntries * sizeof(RootDir)) != numEntries * sizeof(RootDir)) {
        perror("Error reading directory from cluster");
        free(directory);
        return NULL;
    }
    char fileName[12];
    memcpy(fileName, directory->DIR_Name, 10);
    fileName[11] = '\0';
    printf("%s", fileName);
    return directory;
}

void trimFilename(char *fileName) {
    for (int i = 10; i >= 0; i--) {
        if (fileName[i] == ' ' || fileName[i] == '\t') {
            fileName[i] = '\0'; 
        } else {
            break;
        }
    }
}

void printSubDirs(RootDir *rDir, int numOfRootEnt, int fd, uint16_t *FAT, BootSector *bs) {
    
    printf("\n%-20s %-10s %-10s %-12s %-10s %-8s\n", 
           "Filename", "Cluster", "File Size", "Last Modified", "Time", "Attr");
    printf("------------------------------------------------------------------\n");

    for (int i = 0; i < numOfRootEnt; i++) {
        if (rDir[i].DIR_Attr == 0x00) {
            continue;  
        }
        if (rDir[i].DIR_Attr == 0x0F) {
            continue;
        }

        char fileName[12];
        memcpy(fileName, rDir[i].DIR_Name, 11);
        fileName[11] = '\0';  
        trimFilename(fileName);

        if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0) {
            continue;
        }

        uint16_t firstCluster = (rDir[i].DIR_FstClusHI << 16) | rDir[i].DIR_FstClusLO;

        uint8_t day = rDir[i].DIR_WrtDate & 0x1F;
        uint8_t month = (rDir[i].DIR_WrtDate >> 5) & 0x0F;
        uint8_t hour = (rDir[i].DIR_WrtTime >> 11) & 0x1F;
        uint8_t minute = (rDir[i].DIR_WrtTime >> 5) & 0x3F;
        uint8_t second = (rDir[i].DIR_WrtTime & 0x1F) * 2;

        printf("%-20s %-10u %-10u %02u/%02u %02u:%02u:%02u ",
               fileName, 
               firstCluster, 
               rDir[i].DIR_FileSize,
               day, month,  
               hour, minute, second  
        );

        printf("%c", (rDir[i].DIR_Attr & 0x20) ? 'A' : '-');  
        printf("%c", (rDir[i].DIR_Attr & 0x10) ? 'D' : '-');  
        printf("%c", (rDir[i].DIR_Attr & 0x08) ? 'V' : '-');  
        printf("%c", (rDir[i].DIR_Attr & 0x04) ? 'S' : '-'); 
        printf("%c", (rDir[i].DIR_Attr & 0x02) ? 'H' : '-');  
        printf("%c\n", (rDir[i].DIR_Attr & 0x01) ? 'R' : '-'); 

        if (rDir[i].DIR_Attr & 0x10) { 
            readDirs(&rDir[i], fd, FAT, bs);
        }
    }
}

void readDirs(RootDir *rDir, int fd, uint16_t *FAT, BootSector *bs) {
    if (!(rDir->DIR_Attr & 0x10)) { 
        printf("Error: Not a directory.\n");
        return; 
    }

    uint16_t startingClusterNum = (rDir->DIR_FstClusHI << 16) | rDir->DIR_FstClusLO;
    if (startingClusterNum < 2) {
        printf("Error: Invalid cluster number for directory.\n");
        return;
    }

    char dirName[12];
    memcpy(dirName, rDir->DIR_Name, 11);
    dirName[11] = '\0';
    trimFilename(dirName);

    printf("\n Reading: %s\n", dirName);

    RootDir *subDir = readDirectoryFromCluster(fd, bs, startingClusterNum);
    if (!subDir) {
        printf("Error reading directory contents.\n");
        return;
    }

    printSubDirs(subDir, bs->BPB_RootEntCnt, fd, FAT, bs);

    free(subDir);
}

void printRootDir(RootDir *rDir, int numOfRootEnt, int fd, uint16_t *FAT, BootSector *bs) {
    char longFileName[256] = {0};  
    LinkedListString *longFileNameList = NULL;  

    for (int i = 0; i < numOfRootEnt; i++) {
        if (rDir[i].DIR_Attr == 0x00) {
            continue;  
        }
        if(rDir[i].DIR_Attr == 0x10){
            readDirs(&rDir[i], fd, FAT, bs);
        }

        if (rDir[i].DIR_Attr == 0x0F) {
            LongFileNameRootDir *lfn = (LongFileNameRootDir *)&rDir[i];
            
            if (lfn->LDIR_Ord & 0x40) {
                memset(longFileName, 0, sizeof(longFileName));
                freeLinkedList(&longFileNameList); 
            }

            char temp[14] = {0}; 
            extractLFNChars(temp, lfn->LDIR_Name3, 4);
            addStringToStartOfList(&longFileNameList, temp);
            
            extractLFNChars(temp, lfn->LDIR_Name2, 12);
            addStringToStartOfList(&longFileNameList, temp);
            
            extractLFNChars(temp, lfn->LDIR_Name1, 10);
            addStringToStartOfList(&longFileNameList, temp);

            continue;
        }

        char shortFileName[12];
        memcpy(shortFileName, rDir[i].DIR_Name, sizeof(rDir[i].DIR_Name));
        shortFileName[11] = '\0';

        printf("\n%-20s %-10s %-10s %-12s %-10s %-8s %-40s\n",
            "8.3 Filename", "Cluster", "File Size", "Last Modified", "Time", "Attr", "Filename");
        printf("---------------------------------------------------------------------------------------------------------------------\n");

        char longFileNameBuffer[256] = "(None)";  

        if (longFileNameList) {
        LinkedListString *currentNode = longFileNameList;
        longFileNameBuffer[0] = '\0'; 
        while (currentNode) {
            strcat(longFileNameBuffer, currentNode->string); 
            currentNode = currentNode->Next;
        }
        } else {
        strcpy(longFileNameBuffer, shortFileName);
        }

        uint16_t firstCluster = (rDir[i].DIR_FstClusHI << 16) | rDir[i].DIR_FstClusLO;
        uint16_t year = ((rDir[i].DIR_WrtDate >> 9) & 0x7F) + 1980;
        uint8_t month = (rDir[i].DIR_WrtDate >> 5) & 0x0F;
        uint8_t day = rDir[i].DIR_WrtDate & 0x1F;
        uint8_t hour = (rDir[i].DIR_WrtTime >> 11) & 0x1F;
        uint8_t minute = (rDir[i].DIR_WrtTime >> 5) & 0x3F;
        uint8_t second = (rDir[i].DIR_WrtTime & 0x1F) * 2;

        printf("%-20s %-10u %-10u %02u/%02u/%04u %02u:%02u:%02u  ",
            shortFileName,
            firstCluster, 
            rDir[i].DIR_FileSize,
            day, month, year,  
            hour, minute, second
        );

        printf("%-2c", (rDir[i].DIR_Attr & 0x20) ? 'A' : '-');
        printf("%-2c", (rDir[i].DIR_Attr & 0x10) ? 'D' : '-');
        printf("%-2c", (rDir[i].DIR_Attr & 0x08) ? 'V' : '-');
        printf("%-2c", (rDir[i].DIR_Attr & 0x04) ? 'S' : '-');
        printf("%-2c", (rDir[i].DIR_Attr & 0x02) ? 'H' : '-');
        printf("%-2c", (rDir[i].DIR_Attr & 0x01) ? 'R' : '-');

        printf("%-40s\n", longFileNameBuffer);

        freeLinkedList(&longFileNameList);
    }
}


