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

int main()
{
	int val, retval;
	int fd = open(TWB_MEM_DEV_0, O_RDONLY);
	if (fd < 0) {
		printf("Cannot open device file\n");
		exit(-1);
	}

	/* get the mem buffer size */
	retval = ioctl(fd, TWBMEM_GETBUFFSIZE, &val);
	if (retval < 0) {
		printf("Failed reading driver buffer size\n");
		exit(-2);
	}

	printf("Driver buffer size = %d\n", val);
	close(fd);
	
	fd = open(TWB_MEM_DEV_0, O_RDWR);
	if (fd < 0) {
		printf("Cannot open device file\n");
		exit(-1);
	}

	/* change the buffer size to 2048 */
	val = 2048;
	retval = ioctl(fd, TWBMEM_SETBUFFSIZE, &val);
	if (retval < 0) {
		printf("Failed modifying driver buffer size; error = %s\n", strerror(errno));
		exit(-2);
	}

	/* get the mem buffer size again to verify */
	retval = ioctl(fd, TWBMEM_GETBUFFSIZE, &val);
	if (retval < 0) {
		printf("Failed reading driver buffer size\n");
		exit(-2);
	}

	printf("Driver buffer size = %d\n", val);
	return 0;
}
