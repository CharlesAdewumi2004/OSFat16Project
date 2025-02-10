#include "linkedList.h"
#include <stdio.h>
#include <stdlib.h>

int addElementToList(LinkedList **root, int newNum){
    LinkedList *newNode = malloc(sizeof(LinkedList));
    if (newNode == NULL) {
        perror("failed to allocate memory");
        return 1;
    }
    newNode->clusterNum = newNum;
    newNode->Next = NULL;
    if (*root == NULL) {
       *root = newNode;
       return 0; 
    }
    LinkedList *currentPointer = *root;
    while (currentPointer->Next != NULL) {
        currentPointer = currentPointer->Next;
    }
    currentPointer->Next = newNode;
    return 0;
}