#include "allocator.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    printf("Custom Dynamic Memory Allocator Test\n");
    printf("====================================\n");

    printf("\n1. Allocate three blocks\n");
    int *first = my_malloc(32);
    char *second = my_malloc(64);
    double *third = my_malloc(96);

    if (first == NULL || second == NULL || third == NULL) {
        printf("Allocation failed.\n");
        return 1;
    }

    *first = 1234;
    strcpy(second, "hello allocator");
    *third = 3.14159;
    print_heap_state();

    printf("\n2. Free the middle block\n");
    my_free(second);
    print_heap_state();

    printf("\n3. Allocate a smaller block to trigger splitting\n");
    char *small = my_malloc(16);

    if (small == NULL) {
        printf("Allocation failed.\n");
        return 1;
    }

    strcpy(small, "split");
    print_heap_state();
    printf("Splitting test: a smaller allocation reused the previously free block.\n");

    printf("\n4. Free neighboring blocks to trigger coalescing\n");
    my_free(third);
    my_free(small);
    print_heap_state();
    printf("Coalescing test: adjacent free blocks were merged into a larger block.\n");

    printf("\n5. Reallocate the first block to a larger size\n");
    strcpy((char *)first, "data before realloc");
    first = my_realloc(first, 128);

    if (first == NULL) {
        printf("realloc failed.\n");
        return 1;
    }

    printf("Data after realloc: %s\n", (char *)first);
    print_heap_state();

    printf("\n6. Free the remaining allocation\n");
    my_free(first);
    print_heap_state();

    printf("\nAllocator test completed.\n");

    return 0;
}

