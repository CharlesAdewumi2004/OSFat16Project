struct LinkedList{
    int clusterNum;
    struct LinkedList *Next;
};
typedef struct LinkedList LinkedList;

struct LinkedListString{
    char *string;
    struct LinkedListString *Next;
};
typedef struct LinkedListString LinkedListString;

int addElementToList(LinkedList **root, int newNum);
int addStringToStartOfList(LinkedListString **root, char *string);
void freeLinkedList(LinkedListString **head);
