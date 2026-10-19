#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>

// Global variables for sum and mutexes
long long total_sum = 0;
pthread_mutex_t sum_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rand_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int start;
    int end;
} square_params;

void* square(void *params) {
    square_params *p = (square_params *) params;

    // Generate random sleep time (0-1 second)
    pthread_mutex_lock(&rand_mutex);
    long int r = random();
    pthread_mutex_unlock(&rand_mutex);
    usleep(r % 1000000); // Sleep up to 1 second (1,000,000 microseconds)

    // Compute partial sum of squares
    long partial_sum = 0;
    for (int i = p->start; i <= p->end; i++) {
        partial_sum += i * i;
    }

    // Add partial sum to the global total_sum
    pthread_mutex_lock(&sum_mutex);
    total_sum += partial_sum;
    pthread_mutex_unlock(&sum_mutex);

    free(p); // Free the allocated parameters
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <n> <threadCount>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    int threadCount = atoi(argv[2]);

    // Validate input constraints
    if (n < 10 || n > 1000) {
        fprintf(stderr, "Error: n must be between 10 and 1000 inclusive.\n");
        exit(EXIT_FAILURE);
    }
    if (threadCount < 2 || threadCount > (n / 2) || threadCount > 100) {
        fprintf(stderr, "Error: Number of threads must be between 2 and %d (n/2), maximum 100.\n", n / 2);
        exit(EXIT_FAILURE);
    }

    // Seed the random number generator
    srandom(time(NULL));

    // Allocate memory for thread handles
    pthread_t *threads = malloc(threadCount * sizeof(pthread_t));
    if (!threads) {
        perror("Failed to allocate memory for threads");
        exit(EXIT_FAILURE);
    }

    // Create threads with their respective ranges
    for (int i = 0; i < threadCount; i++) {
        square_params *sp = malloc(sizeof(square_params));
        if (!sp) {
            perror("Failed to allocate parameters");
            exit(EXIT_FAILURE);
        }

        // Calculate start and end indices for this thread
        sp->start = (i * n) / threadCount + 1;
        sp->end = ((i + 1) * n) / threadCount;

        if (pthread_create(&threads[i], NULL, square, sp) != 0) {
            fprintf(stderr, "Thread creation failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < threadCount; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Thread join failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // Cleanup and output
    free(threads);
    printf("Sum of squares from 1 to %d: %lld\n", n, total_sum);

    return EXIT_SUCCESS;
}
