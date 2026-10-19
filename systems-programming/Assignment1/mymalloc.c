#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
Preprocessor items, such as
    - Size of heap we're working with
    - Size of header we're working with
    - Rounding up size for 8-byte alignment
    - The global array, as per fig. 2 of p1.pdf
    - Check to see if heap has been created
*/
#define MEMLENGTH 4096
#define HEADERLENGTH 8
#define ROUND(x) (((x) + 7) & ~7)

static union {
  char bytes[MEMLENGTH];
  double not_used;
} heap;

static int check = 0;

/*
Helper functions, including
    - initialize() - initialize the heap, currently has no action if already
initialized
    - detectLeaks() - runs at exit, returns how many bytes were allocated in how
many objects
*/

void detectLeaks(void);

void initialize(void) {
  if (!check) {
    check = 1;
    *((size_t *)&heap.bytes[0]) = MEMLENGTH;
    atexit(detectLeaks);
  }
}

void detectLeaks(void) {
  int bytesLeaked = 0;
  int objCreated = 0;
  int i = 0;
  while (i < MEMLENGTH) {
    size_t currentHeader = *((size_t *)&heap.bytes[i]);
    size_t realSize = currentHeader & ~1;
    if (realSize == 0)
      break;
    if (currentHeader & 1) {
      bytesLeaked += realSize - HEADERLENGTH;
      objCreated += 1;
    }
    i += realSize;
  }
  if (bytesLeaked > 0)
    fprintf(stderr, "mymalloc: %i bytes leaked in %i objects.\n", bytesLeaked,
            objCreated);
}

/*
mymalloc(), implemented by Victor Peng, vmp134
*/
void *mymalloc(size_t size, char *file, int line) {

  // Setup of initial variables and the heap
  void *ret = NULL;
  size_t neededBytes = HEADERLENGTH + ROUND(size);
  initialize();

  // Guard
  if (neededBytes <= 0 || neededBytes > (MEMLENGTH - HEADERLENGTH)) {
    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size,
            file, line);
    return ret;
  }

  // Loop to check for free/unallocated memory
  // Since everything will be 8-byte aligned, the 3 LSBs are unused.
  // We will therefore use the first LSB (representing 2^0) to represent free
  // (0) or allocated (1).
  int i = 0;
  while (i < MEMLENGTH) {
    size_t currentHeader = *((size_t *)&heap.bytes[i]);

    // Branch to handle allocated memory
    if (currentHeader & 1) {
      i += (currentHeader & ~1);
    }

    // Branch to handle allocate, then split
    else if (neededBytes <= (currentHeader & ~1)) {
      *((size_t *)&heap.bytes[i]) = (neededBytes | 1);
      ret = &heap.bytes[(i + HEADERLENGTH)];

      // EXACT byte usage
      if (neededBytes == (currentHeader & ~1))
        return ret;

      // Not enough room for whole block after split (8 bytes)
      if (currentHeader - neededBytes < 16) {
        *((size_t *)&heap.bytes[i]) = (currentHeader | 1);
        return ret;
      }

      // Split
      i += neededBytes;
      *((size_t *)&heap.bytes[i]) = currentHeader - neededBytes;
      return ret;
    }

    // Branch to handle unallocated memory that's too small
    else {
      i += currentHeader;
    }
  }

  // Failure case
  fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file,
          line);
  return ret;
}

/*
myfree(), implemented by Samuel Habib, smh389
*/

void myfree(void *ptr, char *file, int line) {
  if (ptr == NULL) {
    return;
  }

  initialize();

  char *p = (char *)ptr;

  // bounds check
  if (p < &heap.bytes[0] || p >= &heap.bytes[MEMLENGTH]) {
    fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
    exit(2);
  }

  int i = 0;
  int prev = -1;

  while (i < MEMLENGTH) {
    size_t currentHeader = *((size_t *)&heap.bytes[i]);
    size_t currentSize = currentHeader & ~1;

    if (currentSize == 0) {
      break;
    }

    char *chunkStart = &heap.bytes[i];
    char *payloadStart = &heap.bytes[i + HEADERLENGTH];
    char *chunkEnd = &heap.bytes[i + currentSize];

    if (p == payloadStart) {
      //* Double free check
      if (!(currentHeader & 1)) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
      }

      // Mark current chunk free
      *((size_t *)&heap.bytes[i]) = currentSize;

      // Coalesce with next free chunks
      while ((i + (int)currentSize) < MEMLENGTH) {
        size_t nextHeader = *((size_t *)&heap.bytes[i + currentSize]);
        size_t nextSize = nextHeader & ~1;

        if (nextSize == 0 || (nextHeader & 1)) {
          break;
        }

        currentSize += nextSize;
        *((size_t *)&heap.bytes[i]) = currentSize;
      }

      // Coalesce with previous free chunk
      if (prev != -1) {
        size_t prevHeader = *((size_t *)&heap.bytes[prev]);
        size_t prevSize = prevHeader & ~1;

        if (!(prevHeader & 1)) {
          *((size_t *)&heap.bytes[prev]) = prevSize + currentSize;
        }
      }

      return;
    }

    // Pointer is inside a chunk but not at payload start
    if (p >= chunkStart && p < chunkEnd) {
      fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);

      exit(2);
    }

    prev = i;
    i += (int)currentSize;
  }

  // Pointer was not found in any chunk
  fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
  exit(2);
}
