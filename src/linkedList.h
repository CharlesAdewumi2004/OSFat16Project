struct LinkedList{
    int clusterNum;
    struct LinkedList *Next;
};
typedef struct LinkedList LinkedList;

int addElementToList(LinkedList **root, int newNum);