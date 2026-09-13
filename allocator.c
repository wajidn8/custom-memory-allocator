#include "allocator.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define HEAP_SIZE (64 * 1024)
#define ALIGNMENT sizeof(void *)

/*
 * The heap is a fixed byte array owned by this allocator.
 * This keeps the project self-contained and avoids depending on malloc().
 */
static unsigned char memory_pool[HEAP_SIZE];

/*
 * Each block starts with a header. The header stores the size of the user
 * area, whether the block is free, and a pointer to the next block.
 *
 * Memory looks like this:
 *
 * [header][user data][header][user data]...
 */
typedef struct BlockHeader {
    size_t size;
    int is_free;
    struct BlockHeader *next;
} BlockHeader;

static BlockHeader *heap_head = NULL;

/*
 * Round a requested size up to the machine word size so returned user
 * pointers have basic word alignment.
 */
static size_t align_size(size_t size)
{
    size_t remainder = size % ALIGNMENT;

    if (remainder == 0) {
        return size;
    }

    return size + (ALIGNMENT - remainder);
}

/*
 * Return the address immediately after a block header.
 * This is the pointer that the caller is allowed to use as user memory.
 */
static void *user_area(BlockHeader *block)
{
    return (unsigned char *)block + sizeof(BlockHeader);
}

/*
 * Find the first free block that is large enough for the requested size.
 * This is called a first-fit search because the first suitable block is used.
 */
static BlockHeader *find_free_block(size_t size)
{
    BlockHeader *current = heap_head;

    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

/*
 * Split a large free block into an allocated block and a smaller free block.
 * We only split when the remaining space can hold another header plus some data.
 */
static void split_block(BlockHeader *block, size_t requested_size)
{
    size_t minimum_remaining = sizeof(BlockHeader) + ALIGNMENT;

    if (block->size < requested_size + minimum_remaining) {
        return;
    }

    /*
     * The new header is placed after the requested user area.
     *
     * Before:
     * [header][---------------- large free area ----------------]
     *
     * After:
     * [header][requested area][header][---- remaining free area ----]
     */
    BlockHeader *new_block = (BlockHeader *)
        ((unsigned char *)user_area(block) + requested_size);

    new_block->size = block->size - requested_size - sizeof(BlockHeader);
    new_block->is_free = 1;
    new_block->next = block->next;

    block->size = requested_size;
    block->next = new_block;
}

/*
 * Ask the allocator's backing pool for a new block.
 * Since this project uses a fixed array as the backing heap, this means
 * placing a new block immediately after the last block already in use.
 */
static BlockHeader *extend_heap(size_t size)
{
    unsigned char *pool_end = memory_pool + HEAP_SIZE;
    unsigned char *new_block_address;

    if (heap_head == NULL) {
        new_block_address = memory_pool;
    } else {
        BlockHeader *last_block = heap_head;

        while (last_block->next != NULL) {
            last_block = last_block->next;
        }

        new_block_address = (unsigned char *)user_area(last_block) + last_block->size;
    }

    if (new_block_address + sizeof(BlockHeader) + size > pool_end) {
        return NULL;
    }

    BlockHeader *new_block = (BlockHeader *)new_block_address;

    new_block->size = size;
    new_block->is_free = 0;
    new_block->next = NULL;

    if (heap_head == NULL) {
        heap_head = new_block;
    } else {
        BlockHeader *last_block = heap_head;

        while (last_block->next != NULL) {
            last_block = last_block->next;
        }

        last_block->next = new_block;
    }

    return new_block;
}

/*
 * Merge adjacent free blocks. Coalescing reduces fragmentation by turning
 * neighboring free blocks into one larger free block.
 *
 * Before:
 * [FREE 32][FREE 64][USED 16]
 *
 * After:
 * [FREE 32+header+64][USED 16]
 */
static void coalesce(void)
{
    BlockHeader *current = heap_head;

    while (current != NULL && current->next != NULL) {
        BlockHeader *next_block = current->next;

        if (current->is_free && next_block->is_free) {
            current->size += sizeof(BlockHeader) + next_block->size;
            current->next = next_block->next;
            continue;
        }

        current = current->next;
    }
}

/*
 * Allocate a block from the custom heap. The allocator first searches for a
 * reusable free block. If none is found, it extends the backing memory pool.
 */
void *my_malloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    size = align_size(size);

    BlockHeader *block = find_free_block(size);

    if (block != NULL) {
        split_block(block, size);
        block->is_free = 0;
        return user_area(block);
    }

    block = extend_heap(size);

    if (block == NULL) {
        return NULL;
    }

    return user_area(block);
}

/*
 * Mark a block as free and then coalesce adjacent free blocks.
 * The user pointer is converted back to its header by moving backward
 * by one BlockHeader.
 */
void my_free(void *ptr)
{
    if (ptr == NULL) {
        return;
    }

    BlockHeader *block = (BlockHeader *)((unsigned char *)ptr - sizeof(BlockHeader));

    block->is_free = 1;
    coalesce();
}

/*
 * Resize an existing allocation. If the current block is already large enough,
 * nothing needs to move. If the next block is free and gives enough space, the
 * allocation grows in place. Otherwise a new block is allocated and the old
 * contents are copied before the old block is freed.
 */
void *my_realloc(void *ptr, size_t new_size)
{
    if (ptr == NULL) {
        return my_malloc(new_size);
    }

    if (new_size == 0) {
        my_free(ptr);
        return NULL;
    }

    new_size = align_size(new_size);

    BlockHeader *block = (BlockHeader *)((unsigned char *)ptr - sizeof(BlockHeader));

    if (block->size >= new_size) {
        return ptr;
    }

    if (block->next != NULL && block->next->is_free) {
        size_t combined_size = block->size + sizeof(BlockHeader) + block->next->size;

        if (combined_size >= new_size) {
            BlockHeader *next_block = block->next;

            block->size = combined_size;
            block->next = next_block->next;

            split_block(block, new_size);
            block->is_free = 0;

            return ptr;
        }
    }

    void *new_ptr = my_malloc(new_size);

    if (new_ptr == NULL) {
        return NULL;
    }

    memcpy(new_ptr, ptr, block->size);
    my_free(ptr);

    return new_ptr;
}

/*
 * Print every block in the allocator so allocation, splitting, and coalescing
 * can be observed directly while running the test program.
 */
void print_heap_state(void)
{
    BlockHeader *current = heap_head;
    int block_number = 0;

    printf("\n--- Heap state ---\n");

    if (current == NULL) {
        printf("Heap is empty.\n");
        return;
    }

    while (current != NULL) {
        printf("Block %d: address=%p, size=%zu, %s\n",
               block_number,
               (void *)current,
               current->size,
               current->is_free ? "FREE" : "USED");

        current = current->next;
        block_number++;
    }

    printf("------------------\n");
}

