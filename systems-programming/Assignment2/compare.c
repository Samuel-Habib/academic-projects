#include "compare.h"

// Uses binary search to find word insertion point
void insert(struct fileData *file, char *newWordName) {
  // Initial Variables, adding 1 to totalWords since we insert
  file->totalWords++;
  int low = 0;
  int high = file->uniqueWords - 1;
  int med = 0;
  int cmp = 0;

  // Binary Search - we find the word
  while (low <= high) {
    med = low + ((high - low) / 2);
    cmp = strcmp(newWordName, file->words[med].name);

    if (cmp == 0) {
      file->words[med].count++;
      free(newWordName);
      return;
    } else if (cmp < 0) {
      high = med - 1;
    } else {
      low = med + 1;
    }
  }

  // Unique Word - first we check if capacity is large enough
  if (file->uniqueWords >= file->capacity) {
    // realloc and error checking
    struct word *temp =
        realloc(file->words, file->capacity * 2 * sizeof(struct word));
    if (!temp) {
      perror("realloc");
      exit(EXIT_FAILURE);
    }

    // variable updating
    file->words = temp;
    file->capacity *= 2;
  }

  // Unique Word - then we shift (if not at end) and insert
  if (low < file->uniqueWords)
    memmove(&file->words[low + 1], &file->words[low],
            (file->uniqueWords - low) * sizeof(struct word));
  file->words[low].name = newWordName;
  file->words[low].count = 1;
  file->words[low].frequency = 0.0;
  file->uniqueWords++;
}

// Creates the fileData struct for one file
struct fileData *create(char *path) {
  // fileData initialization
  struct fileData *file = malloc(sizeof(struct fileData));
  file->name = strdup(path);
  file->uniqueWords = 0;
  file->totalWords = 0;
  file->capacity = 10;
  file->words = malloc(10 * sizeof(struct word));

  // File variables
  char buf[BUFFERLENGTH];
  size_t bytesRead = 0;
  int fd = open(path, O_RDONLY);

  char *temp = malloc(64 * sizeof(char));
  int tempCapacity = 64;
  int tempLength = 0;

  // Tokenizing
  while ((bytesRead = read(fd, buf, BUFFERLENGTH)) > 0) {
    for (size_t i = 0; i < bytesRead; i++) {
      char readCharacter = buf[i];

      if (readCharacter == '-' || isalnum(readCharacter)) {
        if (tempLength >= tempCapacity) {
          tempCapacity *= 2;
          char *temporaryTemp = realloc(temp, tempCapacity);
          if (!temporaryTemp) {
            perror("realloc");
            exit(EXIT_FAILURE);
          }
          temp = temporaryTemp;
        }
        temp[tempLength] = tolower(readCharacter);
        tempLength++;
      } else if (tempLength > 0) {
        temp[tempLength] = '\0';
        insert(file, strdup(temp));
        tempLength = 0;
      }
    }
  }

  // Final Word Check
  if (tempLength > 0) {
    temp[tempLength] = '\0';
    insert(file, strdup(temp));
  }

  free(temp);
  close(fd);
  return file;
}

// Checks if a string has a suffix
int hasSuffix(const char *name, const char *suffix) {
  return (strlen(name) >= strlen(suffix) &&
          strcmp(name + strlen(name) - strlen(suffix), suffix) == 0);
}

// Recursively searches files
void fileSearch(char *path, char ***fileNames, int *i, int *capacity) {
  struct stat s;
  if (stat(path, &s) == -1)
    return;

  // Case where arg is file
  if (S_ISREG(s.st_mode)) {
    if (*i >= *capacity) {
      *capacity *= 2;
      *fileNames = realloc(*fileNames, *capacity * sizeof(char *));
    }
    (*fileNames)[*i] = strdup(path);
    (*i)++;
    return;
  }

  DIR *dir = opendir(path);
  if (!dir)
    return;

  struct dirent *directory;
  while ((directory = readdir(dir)) != NULL) {
    // Ignore dotfiles
    if (directory->d_name[0] == '.')
      continue;

    // full path name
    char fullPath[PATH_MAX];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", path, directory->d_name);

    // Check type
    struct stat st;
    if (stat(fullPath, &st) == -1)
      continue;

    if (S_ISDIR(st.st_mode)) {
      fileSearch(fullPath, fileNames, i, capacity);
    } else if (S_ISREG(st.st_mode)) {
      if (hasSuffix(directory->d_name, ".txt")) {
        if (*i >= *capacity) {
          *capacity *= 2;
          *fileNames = realloc(*fileNames, *capacity * sizeof(char *));
        }
        (*fileNames)[*i] = strdup(fullPath);
        (*i)++;
      }
    }
  }

  closedir(dir);
}

// Calculates Word Frequency Distribution
void wfd(struct fileData *file) {
  if (file->totalWords == 0)
    return;
  for (int i = 0; i < file->uniqueWords; i++) {
    file->words[i].frequency = (double)file->words[i].count / file->totalWords;
  }
}

// Calculates Jenson-Shannon Distance
double jsd(struct fileData *f1, struct fileData *f2) {
  double kld1 = 0.0, kld2 = 0.0;
  int i = 0, j = 0;

  while (i < f1->uniqueWords && j < f2->uniqueWords) {
    int cmp = strcmp(f1->words[i].name, f2->words[j].name);

    if (cmp == 0) {
      double freq1 = f1->words[i].frequency;
      double freq2 = f2->words[j].frequency;
      double mean = (freq1 + freq2) / 2.0;
      kld1 += freq1 * log2(freq1 / mean);
      kld2 += freq2 * log2(freq2 / mean);
      i++;
      j++;
    } else if (cmp < 0) {
      // word only in f1, mean = freq1/2, log2(freq1 / (freq1/2)) = log2(2) = 1
      kld1 += f1->words[i].frequency;
      i++;
    } else {
      kld2 += f2->words[j].frequency;
      j++;
    }
  }

  // drain remaining words
  while (i < f1->uniqueWords) {
    kld1 += f1->words[i].frequency;
    i++;
  }
  while (j < f2->uniqueWords) {
    kld2 += f2->words[j].frequency;
    j++;
  }

  return sqrt(0.5 * kld1 + 0.5 * kld2);
}

// Compares two struct comparisons to ensure descending order
int compare(const void *a, const void *b) {
  struct comparison *c1 = *(struct comparison **)a;
  struct comparison *c2 = *(struct comparison **)b;

  if (c1->totalWords < c2->totalWords)
    return 1;
  if (c1->totalWords > c2->totalWords)
    return -1;
  return 0;
}

// Frees memory for unused fileData structs
void destroyFile(struct fileData *file) {
  for (int i = 0; i < file->uniqueWords; i++) {
    free(file->words[i].name);
  }
  free(file->words);
  free(file->name);
  free(file);
}

// Frees memory for unused comparison structs
void destroyComparison(struct comparison *c) { free(c); }

// Main
int main(int argc, char **argv) {
  if (argc < 2)
    return 0;

  int totalFiles = 0;
  int nameCapacity = 10;
  char **fileNames = malloc(nameCapacity * sizeof(char *));

  // File Search
  for (int i = 1; i < argc; i++) {
    fileSearch(argv[i], &fileNames, &totalFiles, &nameCapacity);
  }

  if (totalFiles < 2) {
    for (int i = 0; i < totalFiles; i++)
      free(fileNames[i]);
    free(fileNames);
    return 0;
  }

  // fileData Creation and Comparison
  struct fileData **files = malloc(totalFiles * sizeof(struct fileData *));
  struct comparison **comps =
      malloc(NC2(totalFiles) * sizeof(struct comparison *));
  int x = 0;
  for (int i = 0; i < totalFiles; i++) {
    files[i] = create(fileNames[i]);
    wfd(files[i]);
    for (int j = 0; j < i; j++) {
      struct comparison *c = malloc(sizeof(struct comparison));
      c->f1 = files[j]->name;
      c->f2 = files[i]->name;
      c->jsd = jsd(files[i], files[j]);
      c->totalWords = files[i]->totalWords + files[j]->totalWords;
      comps[x] = c;
      x++;
    }
  }

  // Sorting the Comparisons in Descending Order
  qsort(comps, NC2(totalFiles), sizeof(struct comparison *), compare);

  // Printed Results
  for (int i = 0; i < NC2(totalFiles); i++) {
    printf("%f %s %s\n", comps[i]->jsd, comps[i]->f1, comps[i]->f2);
  }

  // Memory Cleanup
  for (int i = 0; i < totalFiles; i++)
    free(fileNames[i]);
  free(fileNames);
  for (int i = 0; i < totalFiles; i++) {
    destroyFile(files[i]);
  }
  free(files);
  for (int i = 0; i < NC2(totalFiles); i++) {
    destroyComparison(comps[i]);
  }
  free(comps);

  return EXIT_SUCCESS;
}
