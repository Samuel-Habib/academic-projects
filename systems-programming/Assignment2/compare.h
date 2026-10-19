#ifndef COMPARE_H
#define COMPARE_H

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFERLENGTH 4096
#define NC2(X) (((X) * ((X) - 1)) / 2)

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

struct word {
  char *name;
  int count;
  double frequency;
};

struct fileData {
  char *name;
  struct word *words;
  int uniqueWords;
  int totalWords;
  int capacity;
};

struct comparison {
  char *f1;
  char *f2;
  double jsd;
  int totalWords;
};

void insert(struct fileData *file, char *newWordName);
struct fileData *create(char *path);
int hasSuffix(const char *name, const char *suffix);
void fileSearch(char *path, char ***fileNames, int *i, int *capacity);
void wfd(struct fileData *file);
double jsd(struct fileData *f1, struct fileData *f2);
int compare(const void *a, const void *b);
void destroyFile(struct fileData *file);
void destroyComparison(struct comparison *c);

#endif
