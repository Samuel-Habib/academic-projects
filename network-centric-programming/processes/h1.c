#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char* argv[]) {

	int num_proc = atoi(argv[2]);
	int n = atoi(argv[1]);
	int seg = n / num_proc;

	// Error handling/ validation

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <end_number> <num_processes>\n", argv[0]);
		return EXIT_FAILURE;
	}


	if (n < 10 || n > 1000) {
		fprintf(stderr, "Error: End number must be between 10 and 1000\n");
		return EXIT_FAILURE;
	}



	if (num_proc < 2 || num_proc > n/2 || num_proc > 100) {
		fprintf(stderr, "Error: Number of processes must be between 2 and %d (max 100)\n", n/2);
		return EXIT_FAILURE;
	}

	// Create pipes - one for each process
	int pipes[num_proc][2];
	for (int i = 0; i < num_proc; i++) {
		if (pipe(pipes[i]) == -1) {
			perror("pipe creation failed");
			return EXIT_FAILURE;
		}
	}

	for (int i = 0; i < num_proc; i++) {
		pid_t pid = fork();

		if (pid == -1) {
			perror("fork failed");
			return EXIT_FAILURE;
		}

		if (pid == 0) { // Child process
			
			close(pipes[i][0]);

			// Calculate partial sum
			int start = i * seg + 1;
			int end = (i == num_proc - 1) ? n : (i + 1) * seg;
			int partial_sum = 0;

			for (int j = start; j <= end; j++) {
				partial_sum += j;
			}

			// Write result to pipe
			if (write(pipes[i][1], &partial_sum, sizeof(partial_sum)) == -1) {
				perror("write to pipe failed");
				exit(EXIT_FAILURE);
			}

			close(pipes[i][1]);
			exit(EXIT_SUCCESS);
		}
	}

	// Parent process
	int total_sum = 0;
	int partial_sum;

	for (int i = 0; i < num_proc; i++) {
		// Close write end of pipe
		close(pipes[i][1]);

		// Read partial sum from child
		if (read(pipes[i][0], &partial_sum, sizeof(partial_sum)) == -1) {
			perror("read from pipe failed");
			return EXIT_FAILURE;
		}

		total_sum += partial_sum;
		close(pipes[i][0]);
	}

	// Wait for all child processes to complete
	for (int i = 0; i < num_proc; i++) {
		wait(NULL);
	}

	printf("The sum from 1 to %d is: %d\n", n, total_sum);

	return EXIT_SUCCESS;
}
