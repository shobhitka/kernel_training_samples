#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE     4096

int main ( int argc, char **argv )
{
	int fd;
	char *address1 = NULL;
	char *address2 = NULL;

	fd = open("/dev/twbmem0", O_RDWR);
	if(fd < 0) {
		perror("Open call failed");
		return -1;
	}

	address1 = mmap (NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (address1 == MAP_FAILED) {
		perror("mmap operation failed");
		return -1;
	}

	address2 = mmap (NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (address2 == MAP_FAILED) {
		perror("mmap operation failed");
		return -1;
	}

	printf("1. %s\n", address1);

	/* modify the buffer */
	strcpy(address1, "Hello back from userspace\n");

	/* verify that the change gets reflected */
	printf("2. %s\n", address2);

	munmap(address1, PAGE_SIZE);
	munmap(address2, PAGE_SIZE);

	close(fd);
	return 0;
}
