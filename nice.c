#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main()
{
	int a = 5;
	pid_t pid = fork();
	if (pid == 0) {
		int ret = nice(-10);
		if (ret == -1) {
			printf("Changing nice value failed = %s\n", strerror(errno));
		} else {
			printf ("New priority is set to %d\n", ret);
		}
		a++;
		printf("a = %d\n", a);
		printf("Child Process\n");
		exit(0);
	} else {
		nice(5);
		a++;
		printf("Parent process\n");
		wait(NULL);
		printf("Waited for child process\n");
		printf("a = %d\n", a);
		printf("Ending parent\n");
		exit(0);
	}
}
