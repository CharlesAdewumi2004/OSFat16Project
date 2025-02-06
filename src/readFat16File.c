
#include "readFat16File.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


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
    }
    return bs;
}

u_int16_t *readFat16Fat(int fd, BootSector *bs){
    u_int16_t ReserveSpace = bs->BPB_RsvdSecCnt * bs->BPB_BytsPerSec; //calculates the size of the reserve space before the FAT
    u_int16_t sizeOfFat = bs->BPB_FATSz16 * bs->BPB_BytsPerSec;
    u_int16_t *FAT = malloc(sizeOfFat);
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

void readCluster(int StartingCluster, u_int16_t *FAT) {
    uint16_t cluster = StartingCluster;
    printf("Cluster Chain: ");
    
    while (cluster < 0xFFF8) { 
        printf("%u -> ", cluster);
        cluster = FAT[cluster];  
    }

    printf("EOF\n");
}

void printNAmountOfFatSection(int n, u_int16_t *FAT){
    printf("First %d Enteries\n", n);
    for(int i = 2; i < n+2; i++){
        printf("FAT[%d] → 0x%04X\n", i, FAT[i]);
    }
}

RootDir *readRootDir(int fd, BootSector *bs){
    u_int16_t ReserveSpace = bs->BPB_RsvdSecCnt * bs->BPB_BytsPerSec;
    u_int16_t sizeOfFat = bs->BPB_FATSz16 * bs->BPB_BytsPerSec;
    u_int32_t sizeOfFatSection = sizeOfFat * bs->BPB_NumFATs;

    lseek(fd, (ReserveSpace+sizeOfFatSection + (32*5)), SEEK_SET);

    RootDir *rDir = malloc(512);
    if (rDir == NULL) {
        perror("error allocating memory for root dir");
        return NULL;
    }

    int bytesRead = read(fd, rDir, 512);
    if (bytesRead < 1) {
        perror("error reading root dir");
        return NULL;
    }
    
    return rDir;
}

void printRootDir(RootDir *rDir){
    printf("\n%-12s %-8s %-10s %-10s %-8s %-8s %s\n", 
       "Filename", "Size", "Time", "Date", "Attr", "Cluster", "Type");
    printf("---------------------------------------------------------------------\n");
    printf("%-12.11s %-8u %02u:%02u:%02u %04u-%02u-%02u %-6s %-8u %s\n",
       rDir->DIR_Name,                    // 8+3 formatted filename (max 11 chars)
       rDir->DIR_FileSize,                 // File size in bytes
       (rDir->DIR_WrtTime >> 11) & 0x1F,   // Hour
       (rDir->DIR_WrtTime >> 5) & 0x3F,    // Minutes
       (rDir->DIR_WrtTime & 0x1F) * 2,     // Seconds (stored as 2-second intervals)
       ((rDir->DIR_WrtDate >> 9) & 0x7F) + 1980,  // Year (from 1980)
       (rDir->DIR_WrtDate >> 5) & 0x0F,    // Month
       rDir->DIR_WrtDate & 0x1F,           // Day
       (rDir->DIR_Attr & 0x10) ? "DIR" : "FILE",  // Directory or File
       ((rDir->DIR_FstClusHI << 16) | rDir->DIR_FstClusLO), // Full cluster number
       (rDir->DIR_Attr & 0x10) ? "DIR" : "FILE"); // File Type (DIR/FILE)
}
