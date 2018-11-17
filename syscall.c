#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define NR_TWB_HELLO 335

int main()
{
	char msg[] = "Hello Kernel";

	syscall(NR_TWB_HELLO, msg, strlen(msg));
	printf("System call status: %s\n", strerror(errno));
}
