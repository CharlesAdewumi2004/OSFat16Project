#include "linkedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int addStringToStartOfList(LinkedListString **root, char *string){
    LinkedListString *newNode = malloc(sizeof(LinkedListString)); 
    if (newNode == NULL) {
        perror("failed to allocate memory");
        return 1;
    }
    newNode->string = malloc(strlen(string) + 1);
    if (newNode->string == NULL) {
        perror("failed to allocate memory for string");
        free(newNode); 
        return 1;
    }
    strcpy(newNode->string, string);
    newNode->Next = NULL;
    if (*root == NULL) {
        *root = newNode;
        return 0; 
    }

    newNode->Next = *root;
    *root = newNode;
    return 0;
}

void freeLinkedList(LinkedListString **head) {
    LinkedListString *temp;
    while (*head) {
        temp = *head;
        *head = (*head)->Next;
        free(temp->string);  
        free(temp);          
    }
}

