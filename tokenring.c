#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

int main() {
	pid_t pid, ppid; // do i need ppid?
	int status;

	char* data[1024];

	printf("Enter a message: ");
	scanf("%s", data);
	printf("\nYou gave me: %s", data);

	// if ((pid = fork()) < 0) {
	// 	perror("fork failure");
	// 	exit(1);
	// } else if (pid == 0) {
	// 	printf("Child %ld\n", (long) getpid());
	// } else {
	// 	sleep(1);
	// 	printf("Parent!");
	// }
	return 0;
}