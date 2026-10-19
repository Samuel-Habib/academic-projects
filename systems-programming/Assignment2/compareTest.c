#include "compare.h"
#include <assert.h>

static int passed = 0;
static int failed = 0;

#define TEST(name, cond)                                                       \
  do {                                                                         \
    if (cond) {                                                                \
      printf("PASS: %s\n", name);                                              \
      passed++;                                                                \
    } else {                                                                   \
      printf("FAIL: %s\n", name);                                              \
      failed++;                                                                \
    }                                                                          \
  } while (0)

static struct fileData *emptyFile(const char *name) {
  struct fileData *f = malloc(sizeof(struct fileData));
  f->name = strdup(name);
  f->uniqueWords = 0;
  f->totalWords = 0;
  f->capacity = 10;
  f->words = malloc(10 * sizeof(struct word));
  return f;
}

void test_hasSuffix() {
  TEST("match", hasSuffix("file.txt", ".txt") == 1);
  TEST("no match", hasSuffix("file.c", ".txt") == 0);
  TEST("exact", hasSuffix(".txt", ".txt") == 1);
  TEST("too short", hasSuffix("tx", ".txt") == 0);
}

void test_insert() {
  struct fileData *f = emptyFile("test");

  insert(f, strdup("zebra"));
  insert(f, strdup("apple"));
  insert(f, strdup("mango"));
  insert(f, strdup("apple")); // duplicate

  TEST("uniqueWords", f->uniqueWords == 3);
  TEST("totalWords", f->totalWords == 4);
  TEST("sorted[0]=apple", strcmp(f->words[0].name, "apple") == 0);
  TEST("sorted[1]=mango", strcmp(f->words[1].name, "mango") == 0);
  TEST("sorted[2]=zebra", strcmp(f->words[2].name, "zebra") == 0);
  TEST("duplicate count", f->words[0].count == 2);

  destroyFile(f);
}

void test_wfd() {
  struct fileData *f = emptyFile("test");
  // spam spam spam eggs -> spam: 3, eggs: 1, total: 4
  insert(f, strdup("spam"));
  insert(f, strdup("eggs"));
  insert(f, strdup("spam"));
  insert(f, strdup("spam"));
  wfd(f);

  double spam_freq = -1.0, eggs_freq = -1.0;
  for (int i = 0; i < f->uniqueWords; i++) {
    if (strcmp(f->words[i].name, "spam") == 0)
      spam_freq = f->words[i].frequency;
    if (strcmp(f->words[i].name, "eggs") == 0)
      eggs_freq = f->words[i].frequency;
  }

  TEST("spam=0.75", fabs(spam_freq - 0.75) < 1e-9);
  TEST("eggs=0.25", fabs(eggs_freq - 0.25) < 1e-9);

  destroyFile(f);
}

void test_jsd() {
  // spec example: F1="hi there hi there", F2="hi hi out there"
  // expected JSD ~= 0.3945
  struct fileData *f1 = emptyFile("f1");
  insert(f1, strdup("hi"));
  insert(f1, strdup("there"));
  insert(f1, strdup("hi"));
  insert(f1, strdup("there"));
  wfd(f1);

  struct fileData *f2 = emptyFile("f2");
  insert(f2, strdup("hi"));
  insert(f2, strdup("hi"));
  insert(f2, strdup("out"));
  insert(f2, strdup("there"));
  wfd(f2);

  TEST("spec example ~0.3945", fabs(jsd(f1, f2) - 0.3945) < 0.001);

  // identical files -> JSD = 0
  struct fileData *f3 = emptyFile("f3");
  struct fileData *f4 = emptyFile("f4");
  insert(f3, strdup("hello"));
  insert(f3, strdup("world"));
  insert(f4, strdup("hello"));
  insert(f4, strdup("world"));
  wfd(f3);
  wfd(f4);
  TEST("identical -> 0", fabs(jsd(f3, f4)) < 1e-9);

  // completely disjoint -> JSD = 1
  struct fileData *f5 = emptyFile("f5");
  struct fileData *f6 = emptyFile("f6");
  insert(f5, strdup("aaa"));
  insert(f6, strdup("zzz"));
  wfd(f5);
  wfd(f6);
  TEST("disjoint -> 1", fabs(jsd(f5, f6) - 1.0) < 1e-9);

  destroyFile(f1);
  destroyFile(f2);
  destroyFile(f3);
  destroyFile(f4);
  destroyFile(f5);
  destroyFile(f6);
}

int main() {
  printf("=== hasSuffix ===\n");
  test_hasSuffix();
  printf("=== insert ===\n");
  test_insert();
  printf("=== wfd ===\n");
  test_wfd();
  printf("=== jsd ===\n");
  test_jsd();
  printf("\n%d passed, %d failed\n", passed, failed);
  return failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
