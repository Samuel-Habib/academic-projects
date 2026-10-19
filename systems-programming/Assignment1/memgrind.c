#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "mymalloc.h"

#define MEMLENGTH 4096
#define HEADERLENGTH 8

void test1(void) {
    for (int i = 0; i < 120; i++) {
        char *p = malloc(1);
        free(p);
    }
}

void test2(void) {
    char *pArr[120] = {NULL};
    for (int i = 0; i < 120; i++) {
        pArr[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++) {
        free(pArr[i]);
    }
}

void test3(void) {
    char *pArr[120] = {NULL};
    int i = 0;
    int total = 0;
    while (total < 120) {
        if (rand() % 2) {
            pArr[i] = malloc(1);
            i++;
            total++;
        }
        else {
            if (i > 0) {
                i--;
                free(pArr[i]);
            }
        }
    }
    for (int j = 0; j < i; j++) {
        free(pArr[j]);
    }
}

void test4(void) {
    char *pArr[120] = {NULL};
    for (int i = 0; i < 120; i++) {
        pArr[i] = malloc(1);
    }
    for (int i = 0; i < 120; i+=2) {
        free(pArr[i]);
    }  
    for (int i = 1; i < 120; i+=2) {
        free(pArr[i]);
    }    
}

void test5(void) {
    char *pArr[170] = {NULL};
    for (int i = 0; i < 170; i++) {
        pArr[i] = malloc(16);
    }
    for (int i = 0; i < 170; i++) {
        if (rand() % 2) {
            free(pArr[i]);
            pArr[i] = NULL;
        }
    }
    for (int i = 0; i < 170; i++) {
        if (pArr[i] != NULL) 
            free(pArr[i]);
    }
}

int main (int argc, char **argv) {

    struct timeval start, end;
    gettimeofday(&start, NULL);

    for (int i = 0; i < 50; i++) {
        //Test 1: Allocate + Free 1-byte object 120 times
        test1();

        //Test 2: Allocate 120 1-byte objects then Free
        test2();

        //Test 3: Randomly Allocate and Free objects, with 120 total Allocates
        test3();
        
        //Test 4: Allocate 120 1-byte objects then Free odds then evens (Coalesce)
        test4();

        //Test 5: Allocate 16-byte objects (24 bytes total) spanning MEMLENGTH, then iterate and randomly Free, then Free remaining
        test5();
    }

    gettimeofday(&end, NULL);
    long long startTotal = (long long)start.tv_sec * 1000000LL + start.tv_usec;
    long long endTotal = (long long)end.tv_sec * 1000000LL + end.tv_usec;
    long long meanTime = (endTotal - startTotal)/50;
    
    printf("%lld microseconds taken on average for 50 workloads", meanTime);
    return EXIT_SUCCESS;
}