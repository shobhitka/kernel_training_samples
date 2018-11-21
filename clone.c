#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main()
{
	int a = 5;
	pid_t pid = fork();
	if (pid == 0) {
		a++;
		printf("a = %d\n", a);
		printf("Child Process\n");
		exit(0);
	} else {
		a++;
		printf("Parent process\n");
		wait(NULL);
		printf("Waited for child process\n");
		printf("a = %d\n", a);
	}
}
