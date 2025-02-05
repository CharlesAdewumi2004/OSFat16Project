#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include "readFat16File.h"

#define BOOTSECTORSIZE 512



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
    char buffer[BOOTSECTORSIZE];
    BootSector *bs = malloc(sizeof(BootSector));
    if(bs == NULL){
        perror("error allocating memeory");
        return NULL;
    }

    lseek(fd, 0, SEEK_SET);

    ssize_t bytesRead = read(fd, buffer, BOOTSECTORSIZE);
    if (bytesRead != BOOTSECTORSIZE) {
        printf("incorrect number of bits read: readFat16File.c");
    }

    memcpy(bs, buffer, sizeof(BootSector));
    return bs;

    close(fd);
}

void readFat16Fat(int fd){
    uint16_t fatEntry;
    lseek(fd, BOOTSECTORSIZE, SEEK_SET);  // Move to FAT start

    for (int i= 0 ; i < 1000; i++) {  // Read first 10 entries
        read(fd, &fatEntry, 2);
        printf("Cluster %d → %04X\n", i, fatEntry);
    }
}
