#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include "twb_ioctl.h"

#define TWB_MEM_DEV_0 	"/dev/twbmem0"
#define TWB_MEM_DEV_1 	"/dev/twbmem1"

#define MAX_BUF_SIZE 	1024
char buf[MAX_BUF_SIZE];

int main()
{
	int val, retval, flags;
	int fd;
	fd_set rset;

	fd = open(TWB_MEM_DEV_0, O_RDONLY);
	if (fd < 0) {
		printf("Cannot open device file\n");
		exit(-1);
	}

	/* set flags for non-block operations on fd */
	flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	FD_ZERO(&rset);
	FD_SET(fd, &rset);

	select(fd + 1, &rset, NULL, NULL, NULL);
	if (FD_ISSET(fd, &rset)) {
		memset(buf, 0, MAX_BUF_SIZE);
		retval = read(fd, buf, MAX_BUF_SIZE);
		if (retval < 0) {
			printf("Read Error: %s\n", strerror(errno));
			close(fd);
			exit(-1);
		} if (retval == 0) {
			/* EOF */
		} else
			printf("Data read = %s\n", buf);
	}

	close(fd);

	return 0;
}
