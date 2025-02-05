#include "readFat16File.h"
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

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
    readFat16Fat(fd);
    return 0;
}