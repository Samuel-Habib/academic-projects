#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef REALMALLOC
#include "mymalloc.h"
#endif

#define MEMSIZE 4096
#define HEADERSIZE 8
#define OBJECTS 64
#define OBJSIZE (MEMSIZE / OBJECTS - HEADERSIZE)

static int is_aligned_8(void *p) { return (((uintptr_t)p) & 0x7u) == 0; }

static void pass(const char *name) { printf("PASS: %s\n", name); }

static void fail(const char *name, const char *msg) {
  fprintf(stderr, "FAIL: %s (%s)\n", name, msg);
  exit(1);
}

int main(void) {

#if defined(TEST_ALIGNMENT)

  void *p = malloc(16);
  if (p == NULL)
    fail("alignment", "malloc returned NULL");
  if (!is_aligned_8(p))
    fail("alignment", "pointer not 8-byte aligned");
  free(p);
  pass("alignment");

#elif defined(TEST_OVERLAP)

  char *obj[OBJECTS];

  for (int i = 0; i < OBJECTS; i++) {
    obj[i] = malloc(OBJSIZE);
    if (obj[i] == NULL)
      fail("overlap", "could not allocate all objects");
  }

  for (int i = 0; i < OBJECTS; i++) {
    memset(obj[i], (unsigned char)(i + 1), OBJSIZE);
  }

  for (int i = 0; i < OBJECTS; i++) {
    for (int j = 0; j < OBJSIZE; j++) {
      if ((unsigned char)obj[i][j] != (unsigned char)(i + 1)) {
        fail("overlap", "objects overlap / corruption detected");
      }
    }
  }

  for (int i = 0; i < OBJECTS; i++)
    free(obj[i]);
  pass("overlap");

#elif defined(TEST_REUSE)

  enum { PAYLOAD = 32, MAX_PTRS = 256 };
  char *arr[MAX_PTRS];
  int n = 0;

  while (n < MAX_PTRS) {
    arr[n] = malloc(PAYLOAD);
    if (arr[n] == NULL)
      break;
    n++;
  }

  if (n < 3)
    fail("reuse", "not enough allocations");

  int victim = n / 2;
  char *victim_ptr = arr[victim];
  free(victim_ptr);

  char *p = malloc(PAYLOAD);
  if (p == NULL)
    fail("reuse", "freed memory not reusable");
  if (p != victim_ptr)
    fail("reuse", "did not reuse freed chunk");

  free(p);
  for (int i = 0; i < n; i++) {
    if (i != victim)
      free(arr[i]);
  }

  pass("reuse");

#elif defined(TEST_COALESCE)

  enum { PAYLOAD = 64, MAX_PTRS = 128 };
  char *arr[MAX_PTRS];
  int n = 0;

  while (n < MAX_PTRS) {
    arr[n] = malloc(PAYLOAD);
    if (arr[n] == NULL)
      break;
    n++;
  }

  if (n < 4)
    fail("coalesce", "not enough allocations");

  free(arr[1]);
  free(arr[2]);

  /* 64-byte payload -> 72-byte block (with 8-byte header)
     Request 96 -> ~104 bytes total.
     Requires arr[1] + arr[2] to coalesce. */
  char *big = malloc(96);
  if (big == NULL)
    fail("coalesce", "adjacent free blocks did not coalesce");
  if (big != arr[1])
    fail("coalesce", "coalesced block did not start at arr[1]");

  free(big);
  for (int i = 0; i < n; i++) {
    if (i != 1 && i != 2)
      free(arr[i]);
  }

  pass("coalesce");

#elif defined(TEST_OOM)

  void *p = malloc(100000);
  if (p != NULL)
    fail("oom", "expected NULL for oversized request");
  // allocator should also print malloc: message
  pass("oom");

#elif defined(TEST_BADFREE_STACK)

  int x = 0;
  free(&x);

  fail("badfree_stack", "did not exit on invalid free");

#elif defined(TEST_BADFREE_INTERIOR)

  char *p = malloc(32);
  if (p == NULL)
    fail("badfree_interior", "setup malloc failed");
  free(p + 1);

  fail("badfree_interior", "did not exit on interior free");

#elif defined(TEST_DOUBLE_FREE)

  char *p = malloc(32);
  if (p == NULL)
    fail("double_free", "setup malloc failed");
  free(p);
  free(p);

  fail("double_free", "did not exit on double free");

#elif defined(TEST_LEAK)

  (void)malloc(64);
  (void)malloc(24);
  pass("leak (check leak message printed on exit)");

#else
  fprintf(stderr, "Define one test flag, e.g.:\n"
                  "  -DTEST_ALIGNMENT\n"
                  "  -DTEST_OVERLAP\n"
                  "  -DTEST_REUSE\n"
                  "  -DTEST_COALESCE\n"
                  "  -DTEST_OOM\n"
                  "  -DTEST_BADFREE_STACK\n"
                  "  -DTEST_BADFREE_INTERIOR\n"
                  "  -DTEST_DOUBLE_FREE\n"
                  "  -DTEST_LEAK\n");
  return 1;
#endif

  return 0;
}
