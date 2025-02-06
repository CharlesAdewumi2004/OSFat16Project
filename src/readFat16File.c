
#include "readFat16File.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>


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
