#ifndef _MYMALLOC_H
#define _MYMALLOC_H

#define malloc(X) mymalloc (X, __FILE__, __LINE__)
#define free(X) myfree (X, __FILE__, __LINE__)

void * mymalloc (size_t size, char *file, int line);
void   myfree (void *ptr, char *file, int line);


#endif
