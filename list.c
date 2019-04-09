#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h> 
#include <assert.h>
#include "list.h"
 

struct nodeStruct* List_createNode(void* item){
	struct nodeStruct* node =malloc(sizeof(struct nodeStruct));
	node -> item = item;
	node -> next = NULL;
	return node;
}
void List_destory ( struct nodeStruct **headRef){
	struct nodeStruct* current = *headRef;
	struct nodeStruct* temp = NULL;
	while (current != NULL ){
		temp = current;
		current = current->next;
		free(temp);          
	}
	*headRef = NULL;
	return;
}
void List_insertHead (struct nodeStruct **headRef, struct nodeStruct *node){
	if (*headRef == NULL ){
		*headRef = node;
	}else {
		struct nodeStruct* tmp = *headRef;
		*headRef = node;
		node->next = tmp;
	}
}

struct nodeStruct* List_gettail (struct nodeStruct ** headRef ){
	struct nodeStruct * current = *headRef;
	while ( current -> next != NULL ){
		current = current -> next;
	} 
	return current;
}

void List_insertTail (struct nodeStruct **headRef, struct nodeStruct *node){
	if (*headRef == NULL){

		*headRef = node;

	}else {
		List_gettail(headRef)-> next= node;
	}
}


int List_countNodes (struct nodeStruct *head){
	int count = 0; 
	if (head == NULL ){
		return count;
	}else {
		struct nodeStruct * current = head;
		while ( current != NULL ){
			count++;
			current = current -> next;
		}
		return count;
	}
}


struct nodeStruct* List_findNode(struct nodeStruct *head, void* item){
	struct nodeStruct * current = head;
	int flag = 0;
	while ( current != NULL  ){
		if ( current -> item == item ){
			flag = 1;
			break;
		}
		current = current -> next;
	}
	if (flag == 1){
		return current;
	}else{
		return NULL;
	}
}


struct nodeStruct* List_deleteNode (struct nodeStruct **headRef, struct nodeStruct *node){
	if ( *headRef == node ){    //check the head 'by hand'.
		*headRef = node->next;
	}else {    //mark the head as 'previous' and the next one as 'current' to form the loop
		struct nodeStruct * previous = *headRef;    
		struct nodeStruct * current = previous->next;
		while ( current != NULL ){  
			if ( current == node ){   //found.
				break;
			}
			previous = previous->next;
			current = current -> next;
		}
		previous -> next = current -> next; //connect the breaking list.
	}
	node->next = NULL;
	return node;
}
void insert (struct nodeStruct **list, struct nodeStruct * newnode){
	//printf("Inserting %d...\n", val);
	struct nodeStruct *headRef = *list;
	void* val = newnode->item;
	if (headRef == NULL){ //empty;
		headRef = newnode;
		newnode->next = NULL;
	}else if (headRef->item >= val ){   // head check;
		struct nodeStruct *tmp = headRef;
		headRef = newnode;
		newnode->next = tmp;
	}else if (List_gettail (&headRef)->item < val){          //tail check;
		struct nodeStruct * tail = List_gettail (&headRef);
		tail->next = newnode;
		newnode->next =NULL;
	}else {                                      
		//printf("Putting %d within the list...\n", val);
		struct nodeStruct* previous = headRef;
		struct nodeStruct* current = headRef-> next;
		while(current!=NULL){

			//printf("while runs\n");
			if (current->item < val){
				previous = current;
				current = current->next;
			}else {
				struct nodeStruct * temp = previous->next;   //insert the new node before current;
				previous->next = newnode;
				newnode->next = temp;
				current = NULL;
			}
		}
	}

	*list = headRef;
}


void List_sort (struct nodeStruct **headRef){  //insertion sort.
	struct nodeStruct * sublist = NULL;    // temp head is created
	struct nodeStruct * current = *headRef;	 //insert from the first one
	while (current != NULL ){
		struct nodeStruct * tmp = current->next;			
		insert (&sublist, current);
		current = tmp;
	}

	*headRef = sublist;   //reasign the list to headRef
}

void print (struct nodeStruct **headRef) {
	struct nodeStruct * current = *headRef;
	int i = 0;
	if (current == NULL){
		puts("(null)");
		return;
	}
	while (current != NULL ){
		printf ("%d: Address: %p, ",i, current->item);
		printf ("actual size: %d, ", *(int*)current->item+8);
		printf ("size: %d\n", *(int*)current->item);
		current = current -> next;
		i++;
	}

}

struct nodeStruct* get_previous(struct nodeStruct **headRef, struct nodeStruct *node){
	if (*headRef == node){
		return *headRef;
	}else {
		struct nodeStruct * previous = *headRef;
		struct nodeStruct * current = (*headRef)->next;
		while (current != NULL ){
			if (current == node){
				return previous;
			}
			previous = previous->next;
			current = current->next;
		}
		puts("get_previous error.\n");
		return NULL;	
	}
}