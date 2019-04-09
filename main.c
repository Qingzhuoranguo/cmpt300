#include <stdio.h>
#include "myalloc.h"

int main(int argc, char* argv[]) {
    //initialize_allocator(100, FIRST_FIT);
    //initialize_allocator(100, BEST_FIT);
    initialize_allocator(100, WORST_FIT);
    printf("Using first fit algorithm on memory size 100\n");
    
    char* ptr[8];
    for (int i = 0; i < 8; i++){
        ptr[i] = allocate (i+1);
        *(ptr[i]) = i+1;
        printf("ptr[%d] is: %d\n", i, *(ptr[i]) );
    }
    print_list();
    for (int i = 0; i < 8; i += 2){
        deallocate (ptr[i]);
    }
    print_list();

    int* fit = allocate (4);
    print_list();
    deallocate(fit);

    print_statistics();
    void* before[100] = {NULL};
    void* after[100] = {NULL};
    int compact_size =  compact_allocation(before, after);
    printf("compact_size is: %d\n", compact_size);
    print_list();
    for (int i = 1; i <= 8; i += 2){
        for (int j = 0; j < compact_size; j ++){
            if (ptr[i] == before[j]){
                ptr[i] = after[j];
            }
        }
    }
    for (int i = 1; i <= 8; i += 2){
         printf("ptr[%d] is: %d\n", i, *(ptr[i]) );
    }
    print_statistics();
    for (int i = 1; i <= 8; i += 2){
        deallocate(ptr[i]);
        print_list();
    }
/*
    int* p[50] = {NULL};
    for(int i=0; i<10; ++i) {
        p[i] = allocate(sizeof(int));
        if(p[i] == NULL) {
            printf("Allocation failed\n");
            continue;
        }
        *(p[i]) = i;
        printf("p[%d] = %p ; *p[%d] = %d\n", i, p[i], i, *(p[i]));
    }
    print_statistics();

    for (int i = 0; i < 10; ++i) {
        if (i % 2 == 0)
            continue;

        if (p[i] == NULL){break;}
        printf("Freeing p[%d]\n", i);
        deallocate(p[i]);
        p[i] = NULL;
    }
    printf("available_memory: %d.\n", available_memory());

    void* before[100] = {NULL};
    void* after[100] = {NULL};
    compact_allocation(before, after);

    print_statistics();

    // You can assume that the destroy_allocator will always be the 
    // last funciton call of main function to avoid memory leak 
    // before exit
*/
    destroy_allocator();

    return 0;
}
