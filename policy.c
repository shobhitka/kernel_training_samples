#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sched.h>

int main(int argc, char *argv[])
{
	int a = 5, ret;
	cpu_set_t set;
	pid_t pid;

	int parentcpu = atoi(argv[1]);
	int childcpu = atoi(argv[2]);

	pid = fork();
	if (pid == 0) {
		printf("Child Process\n");
		struct sched_param param;

		// set cpu affinity
		CPU_SET(childcpu, &set);
		if (sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
			printf("Failed setting child cpu affinity: %s\n", strerror(errno));
		}

		// set scheduler policy as SCHED_FIFO
		param.sched_priority = 10;
		ret = sched_setscheduler(0, SCHED_FIFO, &param);
		if (ret == -1) {
			printf("Changing scheduler policy failed = %s\n", strerror(errno));
		} else {
			printf ("Scheduler policy is set to %d\n", sched_getscheduler(0));
		}
		a++;
		printf("a = %d\n", a);
		for (int i=0; i<1000000000; i++);
		printf("Ending Child Process\n");
		exit(0);
	} else {
		struct sched_param param;
		
		// set cpu affinity
		CPU_SET(childcpu, &set);
		if (sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
			printf("Failed setting child cpu affinity: %s\n", strerror(errno));
		}

		param.sched_priority = 10;
		ret = sched_setscheduler(0, SCHED_FIFO, &param);
		if (ret == -1) {
			printf("Parent Changing scheduler policy failed = %s\n", strerror(errno));
		} else {
			printf ("Parent Scheduler policy is set to %d\n", sched_getscheduler(0));
		}
		a++;
		printf("Parent process\n");
		printf("a = %d\n", a);
		for (int i=0; i<1000000; i++);
		sleep(1);
		//printf("Parent again swapped on CPU\n");
		//wait(NULL);
		//printf("Waited for child process\n");
		printf("Ending parent\n");
		exit(0);
	}
}
