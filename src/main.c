#include "readFat16File.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define IMAGENAME "image/fat16.img"



int main(){
    int fd = open(IMAGENAME, O_RDWR);
    if (fd < 0) {
        perror("error openning file");
        return 1;
    }
    BootSector *bs = readFat16ImageBootSector(fd);
    if (bs == NULL) {
        return 1;
    }
    printBootSector(bs);
    uint16_t *FAT = readFat16Fat(fd, bs);
    if (FAT == NULL) {
        return -1;
    }
    readCluster(5, FAT);
    //printNAmountOfFatSection(100,  FAT);
    RootDir *rDir = readRootDir(fd, bs);
    
    for (int i = 0; i < 20; i++) {
        printRootDir(&rDir[i]);
        printf("------------------------------------------------------\n");
    }
   

    close(fd);
    free(bs);
    free(FAT);
    return 0;
}   