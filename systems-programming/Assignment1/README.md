0. Team Members
Samuel Habib (smh389), Victor Peng (vmp134)

1. Test Plan
    We plan to utilize our tests in memtest.c for correctness testing and memgrind.c for stress testing. 
    We have followed the guidelines in p1.pdf to the best of our ability, and added additional tests to further scrutinize our implementation of mymalloc and myfree. 
    Descriptions of these tests and the item(s) being tested can be seen below.

2. Test Programs
    memtest.c
        - to run memtest with the makefile, `make all` followed by `make run_all`
        - No arguments.
        - Outputs PASS: <test> on success.
        - Outputs FAIL: <test> (<reason>) and exits nonzero on failure.
        - For invalid free or double free tests, the allocator is expected to print the required error message and exit with status 2.
        - Tests:
            `memtest_alignment`
                - Allocates a small object and checks the returned pointer is 8-byte aligned.
            `memtest_overlap`
                - Allocates many objects, fills each with a distinct byte pattern, then verifies the pattern is unchanged (detects overlap/corruption).
            `memtest_reuse`
                - Allocates until full, frees one object, then allocates again and checks the freed block is reused.
            `memtest_coalesce`
                - Allocates until full, frees two adjacent blocks, then requests a larger block that only succeeds if coalescing occurs.
            `memtest_oom`
                - Requests an oversized allocation and checks malloc() returns NULL and prints the required message.
            `memtest_badfree_stack`
                - Calls free() on a stack pointer. Expected: print invalid free message and exit 2.
            `memtest_badfree_interior`
                - Allocates then calls free(p + 1). Expected: print invalid free message and exit 2.
            `memtest_double_free`
                - Allocates, frees, then frees again. Expected: print invalid free message and exit 2.
            `memtest_leak`
                - Allocates objects and exits without freeing. Expected: leak report printed at program exit.
            
    memgrind.c 
        - No arguments.
        - Runs through the 3 standard tasks and 2 other implemented tasks 50 times, returns the microseconds taken on average for 50 workloads (all 5 tasks).
        - Tasks are as follows:
            Test 1: Allocate + Free 1-byte object 120 times
            Test 2: Allocate 120 1-byte objects then Free
            Test 3: Randomly Allocate and Free objects, with 120 total Allocates
            Test 4: Allocate 120 1-byte objects then Free odds then evens. This puts pressure on coalesce, putting it as the focus of this stress test.
            Test 5: Allocate 16-byte objects (24 bytes total) spanning MEMLENGTH, then iterate and randomly Free, then Free remaining. This simulates the allocation of a linked list through all available memory, with random removals of nodes. We scrutinize our programs' ability to maintain fractured memory allocations and coalesce.

3. Further Design Notes
    mymalloc
        - mymalloc uses an 8-byte header that contains the length of the current chunk. The header uses the 2^0 bit to signify free (0) or allocated (1). This also means that the minimum length for a memory block is 16 bytes.
        - During allocation, if a split were to yield an 8-byte chunk leftover, myalloc would give those 8 bytes to the allocated space and skip splitting. This is because a valid memory block needs to be at least 16 bytes.
    myfree
        - myfree(NULL) returns immediately.
        - Pointer validation:
            - If the pointer is outside the heap, inside a chunk but not at the payload start, not found in any chunk, or refers to an already-freed chunk (double free), myfree prints exactly:
                - free: Inappropriate pointer (<file>:<line>)
            - It prints to stderr, includes a newline, and exits with status 2.
        - Freeing and coalescing:
            - Marks the chunk as free by clearing the allocated bit.
            - Coalesces with the previous chunk if it is free.
