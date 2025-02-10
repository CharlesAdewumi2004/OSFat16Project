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
        printf("%s\n",fatFileName);
        printf("%s\n", fileName);
        if (strncmp(fatFileName, fileName, 8) == 0) {
            printf("WE MADE IT\n");
            return &rDir[i]; 
        }
    }
    return NULL; 
}

// Print Root Directory Entry
void printRootDir(RootDir *rDir) {
    char fileName[12];
    memcpy(fileName, rDir->DIR_Name, sizeof(rDir->DIR_Name));
    fileName[11] = '\0';         

    printf("Filename: %s\n", fileName);
    printf("First Cluster: %u\n", (rDir->DIR_FstClusHI << 16) | rDir->DIR_FstClusLO);
    printf("File Size: %u bytes\n", rDir->DIR_FileSize);
    printf("Year: %u\n", ((rDir->DIR_CrtDate >> 9) & 0x7F) + 1980);
    printf("Month: %u\n", (rDir->DIR_CrtDate >> 5) & 0x0F);
    printf("Day: %u\n", (rDir->DIR_CrtDate & 0x1F));
    printf("Hour: %u\n", (rDir->DIR_CrtTime >> 11) & 0x1F);
    printf("Min: %u\n", (rDir->DIR_CrtTime >> 5) & 0x3F);
    printf("Sec: %u\n", (rDir->DIR_CrtTime & 0x1F) * 2);
    printf("Attributes: ");
    printf("%c-", (rDir->DIR_Attr & 0x20) ? 'A' : '-');
    printf("%c-", (rDir->DIR_Attr & 0x10) ? 'D' : '-');
    printf("%c-", (rDir->DIR_Attr & 0x08) ? 'V' : '-');
    printf("%c-", (rDir->DIR_Attr & 0x04) ? 'S' : '-');
    printf("%c-", (rDir->DIR_Attr & 0x02) ? 'H' : '-');
    printf("%c\n", (rDir->DIR_Attr & 0x01) ? 'R' : '-');
}
