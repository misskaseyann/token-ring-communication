#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>

#define READ 0
#define WRITE 1
#define MAX 50

/* Pipe array for file descriptors. */
typedef int Pipe[2];
/* Custom signal handler function. */
void sigHandler(int sig, siginfo_t* si, void* context);

/*
 * TOKEN-RING COMMUNICATION SIM
 * Using processes and pipes.
 * 
 * @author Kasey Stowell
 * @version Fall 2018
 */
int main(int argc, char** argv) {
	// Signal handling.
	struct sigaction sVal;
	sVal.sa_flags = SA_SIGINFO;
	sVal.sa_sigaction = sigHandler;

	// If the argument doesnt exist or is beyond limits, then set to 1.
	int k = (argc > 1) && (atoi(argv[1]) > 0 && atoi(argv[1]) <= MAX) ? atoi(argv[1]) : 1;
	// If second argument doesn't exist or is beyond limits, then set to parent.
	int dest = (argc > 2) && (atoi(argv[2]) >= 0 && atoi(argv[2]) <= k) ? atoi(argv[2]) : 0;
	// Bool for whether or not we actually reached our destination.
	int dest_reached = 0;
	// Parents access to pipe 0 and pipe k.
	int c1_wr, ck_rd;
	// Store parent ID.
	int ppid = getpid();
	printf("Parent with PID %d.\n", ppid);
	sleep(2); // Makes things more elegant.

	// Create pipes array.
	Pipe pipes[k + 1];

	// Initialize parent pipe.
	if (pipe(pipes[0]) < 0) {
		perror("Plumbing problem.\n");
		exit(1);
	}

	printf("Spawning %d processes.\n", k);
	printf("Destination set to process %d.\n", dest);
	printf("Made Pipe 0\n");

	// Make remaining pipes.
	for (int i = 1; i < k + 1; i++) {
		// New pipe.
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
			// Start loop for child.. always reading from its given pipe.
			while (read(pipes[i-1][READ], &msg, MAX) > 0) {
				sleep(1);
				printf("Child %d with PID %d got the message: %s", i, getpid(), msg);
				// If this is the destination we wanted, lets clear the message!
				if (i == dest && dest_reached == 0) {
					printf("Message reached destination! Clearing message from token.\n");
					strcpy(msg, " \n");
					dest_reached = 1;
				}
				// Write message to the next childs pipe.
				write(pipes[i][WRITE], &msg, MAX);
				printf("Child %d with PID %d sending message forward.\n", i, getpid());
			}
			// Clean up.
			close(pipes[i-1][READ]);
			close(pipes[i][WRITE]);
			exit(0);
		}
	} // Parent code.
	// Init the signal handler.
	sigaction(SIGINT, &sVal, NULL);
	// Close pipes that the parent doesn't want.
	for (int i = 1; i < k; i++) {
		// Close write of pipe.
		close(pipes[i][WRITE]);
		// Close read of pipe.
		close(pipes[i][READ]);
	}
	// Close read of the first pipe.
	close(pipes[0][READ]);
	// Close write of the last pipe.
	close(pipes[k][WRITE]);
	// Set parents access to pipe 0 and pipe k.
	c1_wr = pipes[0][WRITE];
	ck_rd = pipes[k][READ];

	// Get message to pass along (we do this only once).
	char payload[MAX];
	printf("Please enter a message to pass around: ");
	fgets(payload, 50, stdin);
	printf("\nYou entered: %sI will now pass it around!\n", payload);

	// Start loop for parent.. always reading from its given pipe.
	while (1) {
		write(c1_wr, payload, MAX);
		read(ck_rd, payload, MAX);
		printf("Parent with PID %d got message: %s", getpid(), payload);
		// Clear message if there is one.
		strcpy(payload, " \n");
	}
	// Clean up.
	close(c1_wr);
	close(ck_rd);

	return 0;
}

/* Custom signal handler function. */
void sigHandler(int sig, siginfo_t* si, void* context) {
	if (SIGINT) {
		sleep(2); // Wrap up processes then announce closing.
		printf(" received an interrupt.\n");
		printf("Closing token-ring.\n");
		exit(0);
	}
}