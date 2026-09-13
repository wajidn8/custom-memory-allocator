#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

void *my_malloc(size_t size);
void my_free(void *ptr);
void *my_realloc(void *ptr, size_t new_size);
void print_heap_state(void);

#endif

