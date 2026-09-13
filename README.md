is it correct?

Yes. Delete it from your project directory with:

rm output.txt

Then check:

ls

You should have only your project files and folders, for example:

README.md
shell.c
.gitignore
screenshots/

Then check Git:

git status

If output.txt was never committed, that's all.

If Git says it was already tracked, remove it from Git too:

git rm output.txt
git commit -m "Remove test output file"
git push

Since you're currently in the middle of fixing the earlier push issue, run git status after deleting it and paste the output here before doing the rebase.

3ff20f25-1bd5-4ce2-aa7c-101c23b3a33b.c
File
now lets do for 2nd project

Yes. Let's put Project 2 — Custom Dynamic Memory Allocator on GitHub now.

We'll use the same clean approach as Project 1.

1. Recommended repository name

Use:

custom-memory-allocator

That's clear and professional.

GitHub description:

A simple dynamic memory allocator implemented in C with first-fit allocation, block splitting, coalescing, and realloc support over a fixed memory pool.

Set it to Public.

Do not create a README or .gitignore on GitHub, because we'll create them locally.

2. Your local project

From your terminal:

cd ~/Documents/vyavya_proj/proj2

Check the files:

ls

You should have approximately:

allocator.c
allocator.h
test_allocator.c
README.md
.gitignore

Your uploaded allocator.c is the allocator implementation, including the fixed 64 KiB pool, BlockHeader, first-fit search, splitting, coalescing, and realloc logic.

3. Before Git, compile and test once
gcc -Wall -Wextra -std=c11 allocator.c test_allocator.c -o allocator_test

Run:

./allocator_test

Make sure the output looks like the one you showed earlier.

Then remove the compiled binary:

rm allocator_test

Your .gitignore will prevent binaries from being committed anyway.

4. Initialize Git
git init

Then:

git add allocator.c allocator.h test_allocator.c README.md .gitignore

Check:

git status

You should not see allocator_test or a.out as files to commit.

5. Make the first commit
git commit -m "Initial implementation of custom memory allocator"
6. Connect your GitHub repo

After you create custom-memory-allocator on GitHub, use:

git branch -M main

Then:

git remote add origin git@github.com:wajidn8/custom-memory-allocator.git

Verify:

git remote -v

You should get:

origin  git@github.com:wajidn8/custom-memory-allocator.git (fetch)
origin  git@github.com:wajidn8/custom-memory-allocator.git (push)
7. Push
git push -u origin main

Since your SSH authentication is already working from Project 1, it should not ask for your GitHub username/password.

Important

Don't upload:

a.out
allocator_test
*.o

Only the source, header, README, .gitignore, and later perhaps a demo screenshot.

Once the GitHub repository is created, tell me "done" and we'll do the local commands one by one, the same way we did for Project 1.

~/Documents/vayavya_proj/project2$ ls
allocator.c  allocator.h  test_allocator.c

i dont have .gitignore and readme give the correct files it should be like written by me not ai



ssh: git@github.com:wajidn8/custom-memory-allocator.git

Yes. Your SSH URL is correct:

git@github.com:wajidn8/custom-memory-allocator.git

I prepared a simple, natural README based on your actual allocator design and tests, without adding claims you haven't demonstrated.

Download README.md

Your .gitignore should contain:

# Compiled programs
allocator_test
a.out

# Object files
*.o

# Other compiled output
*.out

Create it locally:

cd ~/Documents/vayavya_proj/project2
nano .gitignore

Paste the above, save with Ctrl+O, Enter, then Ctrl+X.

Then create/download the README into your project folder. Since your local project already has:

allocator.c
allocator.h
test_allocator.c

the final structure should be:

project2/
├── allocator.c
├── allocator.h
├── test_allocator.c
├── README.md
└── .gitignore
Then do the GitHub setup

First test the project:

gcc -Wall -Wextra -std=c11 allocator.c test_allocator.c -o allocator_test
./allocator_test

Once it works, remove the binary:

rm allocator_test

Then:

git init
git add allocator.c allocator.h test_allocator.c README.md .gitignore
git commit -m "Initial implementation of custom memory allocator"
git branch -M main
git remote add origin git@github.com:wajidn8/custom-memory-allocator.git
git push -u origin main

One correction from earlier: don't use sbrk() in the README, because your actual implementation uses a fixed 64 KiB array as the backing heap. Your allocator.c defines HEAP_SIZE as 64 * 1024 and declares memory_pool as a static byte array.

Once the push succeeds, send me the GitHub repo link and I'll check the final repository structure/readme before we move on. 

README.md
Document

Library
/
README.md
Custom Dynamic Memory Allocator

A small dynamic memory allocator written in C to understand how malloc(), free(), and realloc() work internally.

The allocator manages its own fixed memory pool instead of using the system allocator. Each block has a small header containing its size, whether it is free, and a pointer to the next block.

What it does

The allocator supports:

my_malloc() for allocating memory
my_free() for releasing memory
my_realloc() for resizing an allocation
First-fit search for reusing free blocks
Block splitting
Coalescing adjacent free blocks
Basic word alignment
A test program that prints the state of the managed heap
Memory layout

The allocator keeps blocks in a linked list.

[ header ][ user data ][ header ][ user data ][ header ][ user data ]
     |                         |
     |                         +--> next block
     +--> size / free / next

A block header contains:

size       -> number of bytes available to the user
is_free    -> whether the block is currently free
next       -> pointer to the next block

The pointer returned by my_malloc() points just after the header, so the caller only sees the usable data area.

How allocation works

When my_malloc() is called, the requested size is first rounded up to a basic word-aligned size.

Then the allocator searches the existing blocks from the beginning.

It uses first-fit:

Block 0 -> too small
Block 1 -> free and large enough -> use this block
Block 2 -> not checked

If a suitable free block is found, it is reused.

If the free block is much larger than the requested size, the allocator splits it:

Before:

[ HEADER ][                FREE BLOCK                ]

After:

[ HEADER ][ USED ][ HEADER ][      FREE      ]

The second header describes the remaining free part.

If no suitable free block exists, a new block is placed at the end of the allocator's fixed memory pool.

How free and coalescing work

my_free() marks the block as free.

After that, the allocator checks neighboring blocks and merges adjacent free blocks. This is called coalescing.

For example:

Before:

[ USED ][ FREE 32 ][ FREE 64 ][ USED ]

After:

[ USED ][      FREE      ][ USED ]

The two free blocks are combined into one larger block. This helps reduce fragmentation.

The implementation stores blocks in address order, so adjacent blocks in the list are also adjacent in the managed memory area.

How realloc works

my_realloc() handles resizing an existing allocation.

The allocator checks these cases in order:

If the pointer is NULL, it behaves like my_malloc().
If the new size is zero, the old block is freed.
If the current block is already large enough, the same pointer is returned.
If the next block is free and there is enough combined space, the current block is expanded in place.
Otherwise, a new block is allocated, the old data is copied, and the old block is freed.

This means realloc() does not always have to move the data.

Test program

test_allocator.c exercises the main allocator operations.

The tests include:

allocating several blocks of different sizes
freeing blocks in a non-LIFO order
reusing a free block
triggering block splitting
triggering block coalescing
growing an allocation with realloc()
checking that existing data is preserved
printing the heap state after operations

Example output:

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

The exact memory addresses will be different on different runs.

Build and run

Compile the allocator and test program with GCC:

gcc -Wall -Wextra -std=c11 allocator.c test_allocator.c -o allocator_test

Run it:

./allocator_test
Files
custom-memory-allocator/
├── allocator.c
├── allocator.h
├── test_allocator.c
├── README.md
└── .gitignore
Limitations

This is an educational allocator and is intentionally kept small.

It does not provide:

thread safety
best-fit or worst-fit allocation
a dynamically growing OS heap
advanced alignment requirements
protection against an invalid pointer passed to my_free()
the full set of features and optimizations found in production allocators

The backing heap is a fixed 64 KiB byte array, so the allocator can only manage memory inside that pool.

The first-fit strategy is simple and easy to understand, but it is not always the most memory-efficient choice.

Why I built it

The main goal of this project was to understand what happens underneath a normal dynamic memory allocation call: how free blocks are tracked, how memory can be reused, how fragmentation happens, and how an allocator can split and merge blocks.
