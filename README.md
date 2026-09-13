# Custom Dynamic Memory Allocator

A small dynamic memory allocator written in C to understand how `malloc()`, `free()`, and `realloc()` work internally.

The allocator manages its own fixed memory pool instead of using the system allocator. Each block has a small header containing its size, whether it is free, and a pointer to the next block.

## What it does

The allocator supports:

- `my_malloc()` for allocating memory
- `my_free()` for releasing memory
- `my_realloc()` for resizing an allocation
- First-fit search for reusing free blocks
- Block splitting
- Coalescing adjacent free blocks
- Basic word alignment
- A test program that prints the state of the managed heap

## Memory layout

The allocator keeps blocks in a linked list.

```text
[ header ][ user data ][ header ][ user data ][ header ][ user data ]
     |                         |
     |                         +--> next block
     +--> size / free / next
```

A block header contains:

```text
size       -> number of bytes available to the user
is_free    -> whether the block is currently free
next       -> pointer to the next block
```

The pointer returned by `my_malloc()` points just after the header, so the caller only sees the usable data area.

## How allocation works

When `my_malloc()` is called, the requested size is first rounded up to a basic word-aligned size.

Then the allocator searches the existing blocks from the beginning.

It uses **first-fit**:

```text
Block 0 -> too small
Block 1 -> free and large enough -> use this block
Block 2 -> not checked
```

If a suitable free block is found, it is reused.

If the free block is much larger than the requested size, the allocator splits it:

```text
Before:

[ HEADER ][                FREE BLOCK                ]

After:

[ HEADER ][ USED ][ HEADER ][      FREE      ]
```

The second header describes the remaining free part.

If no suitable free block exists, a new block is placed at the end of the allocator's fixed memory pool.

## How free and coalescing work

`my_free()` marks the block as free.

After that, the allocator checks neighboring blocks and merges adjacent free blocks. This is called **coalescing**.

For example:

```text
Before:

[ USED ][ FREE 32 ][ FREE 64 ][ USED ]

After:

[ USED ][      FREE      ][ USED ]
```

The two free blocks are combined into one larger block. This helps reduce fragmentation.

The implementation stores blocks in address order, so adjacent blocks in the list are also adjacent in the managed memory area.

## How realloc works

`my_realloc()` handles resizing an existing allocation.

The allocator checks these cases in order:

1. If the pointer is `NULL`, it behaves like `my_malloc()`.
2. If the new size is zero, the old block is freed.
3. If the current block is already large enough, the same pointer is returned.
4. If the next block is free and there is enough combined space, the current block is expanded in place.
5. Otherwise, a new block is allocated, the old data is copied, and the old block is freed.

This means `realloc()` does not always have to move the data.

## Test program

`test_allocator.c` exercises the main allocator operations.

The tests include:

- allocating several blocks of different sizes
- freeing blocks in a non-LIFO order
- reusing a free block
- triggering block splitting
- triggering block coalescing
- growing an allocation with `realloc()`
- checking that existing data is preserved
- printing the heap state after operations

Example output:

```text
1. Allocate three blocks

Block 0: size=32, USED
Block 1: size=64, USED
Block 2: size=96, USED

2. Free the middle block

Block 0: size=32, USED
Block 1: size=64, FREE
Block 2: size=96, USED

3. Allocate a smaller block to trigger splitting

Block 0: size=32, USED
Block 1: size=16, USED
Block 2: size=24, FREE
Block 3: size=96, USED

4. Free neighboring blocks to trigger coalescing

Block 0: size=32, USED
Block 1: size=184, FREE
```

The exact memory addresses will be different on different runs.

## Build and run

Compile the allocator and test program with GCC:

```bash
gcc -Wall -Wextra -std=c11 allocator.c test_allocator.c -o allocator_test
```

Run it:

```bash
./allocator_test
```

## Files

```text
custom-memory-allocator/
├── allocator.c
├── allocator.h
├── test_allocator.c
├── README.md
└── .gitignore
```

## Limitations

This is an educational allocator and is intentionally kept small.

It does not provide:

- thread safety
- best-fit or worst-fit allocation
- a dynamically growing OS heap
- advanced alignment requirements
- protection against an invalid pointer passed to `my_free()`
- the full set of features and optimizations found in production allocators

The backing heap is a fixed 64 KiB byte array, so the allocator can only manage memory inside that pool.

The first-fit strategy is simple and easy to understand, but it is not always the most memory-efficient choice.

## Why I built it

The main goal of this project was to understand what happens underneath a normal dynamic memory allocation call: how free blocks are tracked, how memory can be reused, how fragmentation happens, and how an allocator can split and merge blocks.
