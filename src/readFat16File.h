#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "linkedList.h"


typedef struct __attribute__((__packed__)) {
    uint8_t DIR_Name[11];       // 8.3 filename (not null-terminated)
    uint8_t DIR_Attr;           // File attributes
    uint8_t DIR_NTRes;          // Reserved for Windows NT
    uint8_t DIR_CrtTimeTenth;   // Tenth of a second (0-199)
    uint16_t DIR_CrtTime;       // Creation Time (HH:MM:SS in 2s intervals)
    uint16_t DIR_CrtDate;       // Creation Date (YYYY-MM-DD)
    uint16_t DIR_LstAccDate;    // Last Access Date
    uint16_t DIR_FstClusHI;     // **Always 0 in FAT16** (Only used in FAT32)
    uint16_t DIR_WrtTime;       // Last Write Time
    uint16_t DIR_WrtDate;       // Last Write Date
    uint16_t DIR_FstClusLO;     // First cluster number **(Used in FAT16)**
    uint32_t DIR_FileSize;      // File size in bytes
} RootDir;


typedef struct __attribute__((__packed__)) {
    uint8_t BS_jmpBoot[3];
    uint8_t BS_OEMName[8];
    uint16_t BPB_BytsPerSec;
    uint8_t BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;
    uint8_t BS_DrvNum;
    uint8_t BS_Reserved1;
    uint8_t BS_BootSig;
    uint32_t BS_VolID;
    uint8_t BS_VolLab[11];
    uint8_t BS_FilSysType[8];
} BootSector;

typedef struct __attribute__((__packed__)) {
    uint8_t LDIR_Ord; // Order/ position in sequence/ set
    uint8_t LDIR_Name1[ 10 ]; // First 5 UNICODE characters
    uint8_t LDIR_Attr; // = ATTR_LONG_NAME (xx001111)
    uint8_t LDIR_Type; // Should = 0
    uint8_t LDIR_Chksum; // Checksum of short name
    uint8_t LDIR_Name2[ 12 ]; // Middle 6 UNICODE characters
    uint16_t LDIR_FstClusLO; // MUST be zero
    uint8_t LDIR_Name3[ 4 ]; // Last 2 UNICODE characters

}LongFileNameRootDir;

typedef struct{ 
    char fullFileName[255];
    char partialFileName[12];
    LinkedList *clusterChain;
    LinkedListString *longFileName;
    int numOfCluster;
    char *fileContents;
}File;

//BootSector
BootSector *readFat16ImageBootSector(int fd);
void printBootSector( BootSector *bs);

//FAT
u_int16_t *readFat16Fat(int fd, BootSector *bs);
void readCluster(int StartingCluster, u_int16_t *FAT);
void printNAmountOfFatSection(int n, u_int16_t *FAT);
void addClustersToLinkedList(int StartingCluster, uint16_t *FAT, LinkedList *root);

//RootDir
RootDir *readRootDir(int fd, BootSector *bs);
void printRootDir(RootDir *rDir, int numOfRootEnt, int fd, uint16_t *FAT, BootSector *bs);
RootDir *findFileClusters(const char *fileName, RootDir *rDir, int totalEntries);
void extractLFNChars(char *buffer, uint8_t *src, int count);
void readDirs(RootDir *rDir, int fd, uint16_t *FAT, BootSector *bs);
void printSubDirs(RootDir *rDir, int numOfRootEnt, int fd, uint16_t *FAT, BootSector *bs);
void trimFilename(char *fileName);


//file Handling
void convertToFat83(const char *filename, char *fatName);
void getFileName(char inputFileName[255]);
File *openFile(BootSector *bs, RootDir *rDir, uint16_t *FAT);
void closeFile(File *file);
off_t seekFileCluster(BootSector *bs, File *file, int fd);
void readFile(File *file,int fd, BootSector *bs);