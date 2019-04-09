#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "myalloc.h"
#include <string.h>
#include "list.h"
#include <pthread.h>
       
struct Myalloc {
    enum allocation_algorithm aalgorithm;
    int size;
    void* memory;
  	
	struct nodeStruct *used_head; 
	struct nodeStruct *free_head; 

};

struct Myalloc myalloc;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void initialize_allocator(int _size, enum allocation_algorithm _aalgorithm) {
    assert(_size > 0);
    if (_size <= 0){
        puts("Initializer: size smaller than 0.");
        return;
    }
    myalloc.aalgorithm = _aalgorithm;
    myalloc.size = _size;
    myalloc.memory = malloc((size_t)myalloc.size); 
    //initialize content to 0.
    memset ( myalloc.memory, 0, (size_t)myalloc.size );
    //linked list initialization
    myalloc.used_head = NULL; 
    	//append the chunk to free list
    void *node = List_createNode (myalloc.memory);
	List_insertHead (&myalloc.free_head, (struct nodeStruct*)node);
		//store the size of the free chunk
	*(size_t*) ( (myalloc.free_head)-> item ) = _size - 8;

    return;
}

void destroy_allocator() {
    pthread_mutex_lock(&mutex);
    List_destory (&myalloc.used_head);
    List_destory (&myalloc.free_head);
    free(myalloc.memory);
    pthread_mutex_unlock(&mutex);
}

//Sort the free list first, then join contiguous free blocks
//tool function, it is not protected by mutex lock, condition is protected by caller
void contiguous_list (){
	List_sort (&myalloc.free_head);//insertion sort asc order
	struct nodeStruct *current = myalloc.free_head; 
	struct nodeStruct *comparsion = current->next; 
	while (current != NULL && comparsion != NULL){
		void *cur = current->item;
		void *nxt = comparsion->item;
        char size = *(long*)(cur) + 8;
		if ( (void*)( (char*)cur + size ) == nxt ){
			*(size_t*)cur = *(size_t*)cur + *(size_t*)nxt + 8;
			current->next = comparsion->next; //connect the list
			free(comparsion); //physically delete the node
			//increment, current stays the same since there's join
			comparsion = current->next; 
			continue;
		}
		//normal increment
		current = current -> next;
		comparsion =  comparsion->next;
	}
}

void* allocate(int _size) {
    pthread_mutex_lock(&mutex);
    void* ptr = NULL;
    if (_size <= 0 ){
    	puts("error.");
        pthread_mutex_unlock(&mutex);
    	return ptr;
    }
    if (myalloc.free_head == NULL){ //no initialization
    	puts("Allocator: free memory list empty.");
        pthread_mutex_unlock(&mutex);
    	return ptr;
    }
    void *target = NULL;
    struct nodeStruct *current = myalloc.free_head;
    if (myalloc.aalgorithm == FIRST_FIT){
    	while (current != NULL){
    		if (  *(long*)(current->item) >= _size ){
  				target = (void*)(current);
                break;
    		}
    		current = current -> next;
    	}
    }else if (myalloc.aalgorithm == BEST_FIT){
    	while (current != NULL){
    		if (  *(long*)(current->item) >= _size ){
    			if (target == NULL){
    				target = (void*)(current);
    			}else {
                    //if theres a better targert
    				if (  *(long*)(current->item) <  *(long*)(((struct nodeStruct*)target)->item) ){
    					target = (void*)(current);
    				}
    			}
    		}
    		current = current -> next;
    	}	
    }else {
    	while (current != NULL){
    		if (  *(long*)(current->item) >= _size ){
    			if (target == NULL){
    				target = (void*)(current);
    			}else {
                    //if theres a better targert
    				if (  *(long*)(current->item) >  *(long*)(((struct nodeStruct*)target)->item) ){
    					target = (void*)(current);
    				}
    			}
    		}
    		current = current -> next;
    	}
    }
    if (target == NULL){
    	printf("Allocator: not enough memory.\n");
        pthread_mutex_unlock(&mutex);
    	return ptr;
    }
    void* list_target = target;
    target = ((struct nodeStruct*)target)->item;

    if ( (*(long*)target-_size) <= 8 ){ //left over smaller than 8
        List_deleteNode (&myalloc.free_head, (struct nodeStruct*)list_target);
        List_insertHead (&myalloc.used_head, (struct nodeStruct*)list_target);
        ptr = (void*) ((char*)target+8);
        pthread_mutex_unlock(&mutex);
        return ptr;
    }

    void* new_node_point = (void*) ((char*) target + 8 + _size);
    *(long*)new_node_point = *(long*)target - _size - 8;

    *(long*)target = _size;

    struct nodeStruct* use_node = List_createNode (target);
    struct nodeStruct* free_node = List_createNode (new_node_point);


    List_deleteNode (&myalloc.free_head, list_target);
    free(list_target);
    List_insertHead (&myalloc.used_head, use_node);
    List_insertHead (&myalloc.free_head, free_node);

    ptr = (void*) ((char*)target+8);
    pthread_mutex_unlock(&mutex);
    return ptr;
}

void print_list(){
    pthread_mutex_lock(&mutex);
	puts("----------\nUsed list:");
	print ( &myalloc.used_head);
	puts("Free list:");
    print ( &myalloc.free_head);
    puts("----------");
    pthread_mutex_unlock(&mutex);
}

void deallocate(void* _ptr) {
    assert(_ptr != NULL);
    pthread_mutex_lock(&mutex);
    if (_ptr == NULL ){
        puts("Error: illegal address.");
        pthread_mutex_unlock(&mutex);
        return;
    }
    struct nodeStruct *temp_node = List_findNode(myalloc.used_head, (void*)((char*)_ptr-8) );
    if (temp_node == NULL){
        puts("deallocate(): invalid pointers: not exsits in memory.");
        pthread_mutex_unlock(&mutex);
        return;
    }
    List_deleteNode (&myalloc.used_head, temp_node);
    List_insertHead (&myalloc.free_head, temp_node);
	contiguous_list ();  //make sure free list is sorted and contiguous
    //_ptr = NULL; //block access from user.
    pthread_mutex_unlock(&mutex);
    return;
}

int compact_allocation(void** _before, void** _after) {
    pthread_mutex_lock(&mutex);
    int compacted_size = 0;
    if (myalloc.free_head == NULL){
        pthread_mutex_unlock(&mutex);
        return 0;
    }
    List_sort(&myalloc.used_head);

    struct nodeStruct *current = myalloc.used_head;
    void *desired_address = myalloc.memory;

    while (current!=NULL){
        //allocated size including the invisible header size.
        int chunk_size = *(long*)(current->item) + 8;
        if ( current->item != desired_address ){
            //store original address
            _before[compacted_size] = current->item;
            //move the chunk
            current->item = desired_address;
            //save desired address
            _after[compacted_size] = current->item;
            //make the actual move
            memmove (_after[compacted_size], _before[compacted_size], chunk_size);
            //ajust pointers to "visible" (without header)
            _after[compacted_size]  = (void*)((char*)(_after[compacted_size])+8);
            _before[compacted_size] = (void*)((char*)(_before[compacted_size])+8);
            //update desired address for next allocated chunk
            desired_address = (void*)( (char*)(current->item) + chunk_size );
            //move to the next chunk
            current = current->next;
            //increment size
            compacted_size ++;
            continue;
        }
        desired_address = (void*)( (char*)(current->item) + chunk_size );
        current = current->next;
    }
    //destory the original free list
    List_destory (&myalloc.free_head);
    //create a new node points to the compacted free chunk
    struct nodeStruct *node = List_createNode(desired_address);
    //append the node 
    List_insertHead (&myalloc.free_head, node);
    //set metadata of free chunk: left size = memory base + size - desired addr - 8
    *(long*)desired_address = ( (char*)myalloc.memory + myalloc.size ) - (char*)desired_address - 8;


    pthread_mutex_unlock(&mutex);
    return compacted_size;
}

int available_memory() {
    pthread_mutex_lock(&mutex);
    int available_memory_size = 0;
    struct nodeStruct *current = myalloc.free_head;
    while (current != NULL){
        available_memory_size += *(long*)(current->item);
        current = current->next;
    }
    pthread_mutex_unlock(&mutex);
    return available_memory_size;
}
int available_memory_cpy() {
    int available_memory_size = 0;
    struct nodeStruct *current = myalloc.free_head;
    while (current != NULL){
        available_memory_size += *(long*)(current->item);
        current = current->next;
    }
    return available_memory_size;
}

int allocated_memory(){
    int available_memory_size = 0;
    struct nodeStruct *current = myalloc.used_head;
    while (current != NULL){
        available_memory_size += *(long*)(current->item);
        current = current->next;
    }
    return available_memory_size;
}
int min_free_chunk (){
    int min = 0;
    struct nodeStruct *current = myalloc.free_head;
    while (current != NULL ){
        if (min == 0){
            min = *(long*)(current->item);
        }
        if ( *(long*)(current->item) < min ){
            min = *(long*)(current->item);
        }
        current = current -> next;
    }
    return min;
}
int max_free_chunk (){
    int max = 0;
    struct nodeStruct *current = myalloc.free_head;
    while (current != NULL ){
        if (max == 0){
            max = *(long*)(current->item);
        }
        if ( *(long*)(current->item) > max ){
            max = *(long*)(current->item);
        }
        current = current -> next;
    }
    return max;
}
void print_statistics() {
    pthread_mutex_lock(&mutex);
    int allocated_size = 0;
    int allocated_chunks = 0;
    int free_size = 0;
    int free_chunks = 0;
    int smallest_free_chunk_size = myalloc.size;
    int largest_free_chunk_size = 0;

    // Calculate the statistics
    allocated_size = allocated_memory();
    allocated_chunks = List_countNodes (myalloc.used_head);
    free_size = available_memory_cpy();
    free_chunks = List_countNodes (myalloc.free_head);
    smallest_free_chunk_size = min_free_chunk();
    largest_free_chunk_size = max_free_chunk();


    printf("Allocated size = %d\n", allocated_size);
    printf("Allocated chunks = %d\n", allocated_chunks);
    printf("Free size = %d\n", free_size);
    printf("Free chunks = %d\n", free_chunks);
    printf("Largest free chunk size = %d\n", largest_free_chunk_size);
    printf("Smallest free chunk size = %d\n", smallest_free_chunk_size);
    pthread_mutex_unlock(&mutex);
}



