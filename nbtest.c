#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "twb_ioctl.h"

#define TWB_MEM_DEV_0 	"/dev/twbmem0"
#define TWB_MEM_DEV_1 	"/dev/twbmem1"

#define MAX_BUF_SIZE 	1024
char buf[MAX_BUF_SIZE];

int main()
{
	int val, retval, flags;
	int fd = open(TWB_MEM_DEV_0, O_RDONLY);
	if (fd < 0) {
		printf("Cannot open device file\n");
		exit(-1);
	}

	/* set flags for non-block operations on fd */
	flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	while (1) {
		retval = read(fd, buf, MAX_BUF_SIZE);
		if (retval < 0 && errno == EAGAIN) {
			printf("No data available, will retry in 1 second\n");
			sleep(1);
		} else if (retval < 0) {
			printf("Read Error: %s\n", strerror(errno));
			close(fd);
			exit(-1);
		} else
			printf("Data read = %s\n", buf);
	}

	close(fd);

	return 0;
}
