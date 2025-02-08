
#include "readFat16File.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <endian.h>


#define BOOTSECTORSIZE 512
#define FATENTRYSIZE 2

void printBootSector(BootSector *bs){    
    printf("OEM Name: %.8s\n", bs->BS_OEMName);
    printf("Bytes per Sector: 0x%04X (%d)\n", bs->BPB_BytsPerSec, bs->BPB_BytsPerSec);
    printf("Sectors per Cluster: 0x%02X (%d)\n", bs->BPB_SecPerClus, bs->BPB_SecPerClus);
    printf("Reserved Sectors: 0x%04X (%d)\n", bs->BPB_RsvdSecCnt, bs->BPB_RsvdSecCnt);
    printf("Number of FATs: 0x%02X (%d)\n", bs->BPB_NumFATs, bs->BPB_NumFATs);
    printf("Root Entry Count: 0x%04X (%d)\n", bs->BPB_RootEntCnt, bs->BPB_RootEntCnt);
    printf("Total Sectors (16-bit): 0x%04X (%d)\n", bs->BPB_TotSec16, bs->BPB_TotSec16);
    printf("Media Descriptor: 0x%02X\n", bs->BPB_Media);
    printf("Sectors per FAT: 0x%04X (%d)\n", bs->BPB_FATSz16, bs->BPB_FATSz16);
    printf("Volume Label: %.11s\n", bs->BS_VolLab);
    printf("File System Type: %.8s\n", bs->BS_FilSysType);
}

BootSector *readFat16ImageBootSector(int fd) {
    BootSector *bs = malloc(sizeof(BootSector));
    if(bs == NULL){
        perror("error allocating memeory");
        return NULL;
    }

    lseek(fd, 0, SEEK_SET);

    ssize_t bytesRead = read(fd, bs, sizeof(*bs));
    if (bytesRead != sizeof(*bs)) {
        printf("incorrect number of bits read");
        free(bs);
        return NULL;
    }
    return bs;
}

uint16_t *readFat16Fat(int fd, BootSector *bs){
    uint16_t ReserveSpace = bs->BPB_RsvdSecCnt * bs->BPB_BytsPerSec; //calculates the size of the reserve space before the FAT
    uint16_t sizeOfFat = bs->BPB_FATSz16 * bs->BPB_BytsPerSec;
    uint16_t *FAT = malloc(sizeOfFat);
    if (FAT == NULL) {
        perror("Failed to allocate memory to FAT");
        return NULL;
    }

    lseek(fd, ReserveSpace, SEEK_SET);

    int byteRead = read(fd, FAT, sizeOfFat);
    if (byteRead < 0) {
        perror("error reading FAT section of image");
        return NULL;
    }
    
    return FAT;
}

void readCluster(int StartingCluster, uint16_t *FAT) {
    uint16_t cluster = StartingCluster;
    printf("Cluster Chain: ");
    
    while (cluster < 0xFFF8) { 
        printf("%u -> ", cluster);
        cluster = FAT[cluster];  
    }

    printf("EOF\n");
}

void printNAmountOfFatSection(int n, uint16_t *FAT){
    printf("First %d Enteries\n", n);
    for(int i = 2; i < n+2; i++){
        printf("FAT[%d] → 0x%04X\n", i, FAT[i]);
    }
}

RootDir *readRootDir(int fd, BootSector *bs) {
    uint32_t rootDirStart = (bs->BPB_RsvdSecCnt + (bs->BPB_NumFATs * bs->BPB_FATSz16)) * bs->BPB_BytsPerSec;
    uint32_t rootDirSize = bs->BPB_RootEntCnt * sizeof(RootDir);

    RootDir *rDir = malloc(rootDirSize);
    if (!rDir) {
        perror("Error allocating memory for Root Directory");
        return NULL;
    }

    lseek(fd, rootDirStart, SEEK_SET);

    ssize_t bytesRead = read(fd, rDir, rootDirSize);
    if (bytesRead != rootDirSize) {
        perror("Error reading Root Directory");
        free(rDir);
        return NULL;
    }

    return rDir;
}

void printRootDir(RootDir *rDir){

    char fileName[12];
    memcpy(fileName, rDir->DIR_Name, sizeof(rDir->DIR_Name));
    fileName[11] = '\0';         

    printf("filename %s\n", fileName);
    printf("first cluser %u\n", (rDir->DIR_FstClusLO));
    //uint32_t DIR_FileSize = le32toh(rDir->DIR_FileSize);
    //printf("file size %d\n", DIR_FileSize);
    printf("Year %u\n", ((rDir->DIR_CrtDate >> 9)& 0x7F) + 1980);
    printf("month %u\n", (rDir->DIR_CrtDate >> 5) & 0x0F);
    printf("day %u\n", (rDir->DIR_CrtDate & 0x1F));
    printf("hour %u\n", (rDir->DIR_CrtTime >> 11) & 0x1F);
    printf("min %u\n", (rDir->DIR_CrtTime >> 5 ) & 0x3F);
    printf("sec %u\n", (rDir->DIR_CrtTime & 0x1F) * 2);
    printf("%u\n", rDir->DIR_Attr);
    printf("%c", (rDir->DIR_Attr & 0x20)  ? 'A': '/');
    printf("%c", (rDir->DIR_Attr & 0x10)  ? 'D': '/');
    printf("%c", (rDir->DIR_Attr & 0x8)  ? 'V': '/');
    printf("%c", (rDir->DIR_Attr & 0x4)  ? 'S': '/');
    printf("%c", (rDir->DIR_Attr & 0x2)  ? 'H': '/');
    printf("%c\n", (rDir->DIR_Attr & 0x1)  ? 'R': '/');
}
