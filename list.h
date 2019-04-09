struct nodeStruct {
    void *item;
    struct nodeStruct *next;
};

//Create a node.
struct nodeStruct* List_createNode(void* item);

void List_destory ( struct nodeStruct **headRef);

//Insert a node to a head pointer.
void List_insertHead (struct nodeStruct **headRef, struct nodeStruct *node);

//Insert a node to the end of the list. Operation needs function gettail();
void List_insertTail (struct nodeStruct **headRef, struct nodeStruct *node);

//Count the number of nodes in a list.
int List_countNodes (struct nodeStruct *head);

//Find a node in a list, return NULL if not found.
struct nodeStruct* List_findNode(struct nodeStruct *head, void* item);

//Delete a specific node.
struct nodeStruct* List_deleteNode (struct nodeStruct **headRef, struct nodeStruct *node);

//Operates insertion sort to a list. Operation needs funtions such as insert();
void List_sort (struct nodeStruct **headRef);


struct nodeStruct* get_previous(struct nodeStruct **headRef, struct nodeStruct *node);

//Helper function. Prints a list.
void print (struct nodeStruct **headRef);