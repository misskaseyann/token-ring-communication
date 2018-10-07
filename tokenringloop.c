#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>

#define READ 0
#define WRITE 1
#define MAX 50

typedef int Pipe[2];

int main(int argc, char** argv) {
	// If the argument doesnt exist or is beyond limits, then set to 1.
	int k = (argc > 1) && (atoi(argv[1]) > 0 && atoi(argv[1]) <= MAX) ? atoi(argv[1]) : 1;
	int c1_wr, ck_rd;
	int ppid = getpid();
	printf("Parent with PID %d.\n", ppid);
	sleep(3); // debug for tree

	// Create pipes array.
	Pipe pipes[k];

	// Initialize parent pipe.
	if (pipe(pipes[0]) < 0) {
		perror("Plumbing problem.\n");
		exit(1);
	}
	printf("Made Pipe 0\n");

	printf("Spawning %d processes.\n", k);
	for (int i = 1; i < k; i++) {
		// Make new pipe.
		if (pipe(pipes[i]) < 0) {
			perror("Plumbing problem.\n");
			exit(1);
		}

		fflush(stdout);
		printf("Made Pipe %d\n", i);

		// Make child.
		pid_t pid = fork();
		if (pid < 0) {
			perror("Fork failed.");
			exit(1);
		} else if (pid == 0) { // Child code.
			// Close pipes that the child doesn't want.
			for (int x = 0; x < i-1; x++) {
				// Close write of pipe.
				close(pipes[x][WRITE]);
				// Close read of pipe.
				close(pipes[x][READ]);
			}
			// Close the write pipe the last process.
			close(pipes[i-1][WRITE]);
			// Close the read pipe of the current process.
			close(pipes[i][READ]);
			// Read in.
			char msg[MAX];
			while (read(pipes[i-1][READ], &msg, MAX) > 0) {
				sleep(1);
				printf("Child %d with PID %d got the message: %s", i, getpid(), msg);
				write(pipes[i][WRITE], &msg, MAX);
				printf("Child %d with PID %d sending message forward.\n", i, getpid());
			}
			close(pipes[i-1][READ]);
			close(pipes[i][WRITE]);
			exit(0);
		}
		// parent code
	}
	// Close pipes that the parent doesn't want.
	for (int i = 1; i < k - 1; i++) {
		// Close write of pipe.
		close(pipes[i][WRITE]);
		// Close read of pipe.
		close(pipes[i][READ]);
	}
	// Close read of the first pipe.
	close(pipes[0][READ]);
	// Close write of the last pipe.
	close(pipes[k - 1][WRITE]);

	c1_wr = pipes[0][WRITE];
	ck_rd = pipes[k - 1][READ];

	char payload[MAX];
	printf("Please enter a message to pass around: ");
	fgets(payload, 50, stdin);
	printf("\nYou entered: %sI will now pass it around!\n", payload);

	while (1) {
		write(c1_wr, payload, MAX);
		read(ck_rd, payload, MAX);
		printf("Parent with PID %d got message: %s", getpid(), payload);
		strcpy(payload, " \n");
	}

	close(c1_wr);
	close(ck_rd);
	return 0;
}