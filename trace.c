#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	int trace_fd = -1;
	int marker_fd = -1;
	int twb_mem_fd = -1;

	trace_fd = open("/sys/kernel/debug/tracing/tracing_on", O_RDWR);
	if (trace_fd < 0) {
		perror("opening trace enable file failed\n");
		exit(-1);
	}

	/* enable tracing */
	write(trace_fd, "1", 1);

	/* open marker file and add a marker */
	marker_fd = open("/sys/kernel/debug/tracing/trace_marker", O_RDWR);
	if (marker_fd < 0) {
		perror("opening marker file failed\n");
		exit(-1);
	}

	write(marker_fd, "TWB_MEM_TRACE", strlen("TWB_MEM_TRACE"));

	/* open twb_mem device and write to it */
	twb_mem_fd = open("/dev/twbmem0", O_WRONLY);
	if (twb_mem_fd < 0) {
		perror("opening twbmem0 failed\n");
		exit(-1);
	}

	/* write to it */
	write(twb_mem_fd, "Hello", strlen("Hello"));

	close(twb_mem_fd);

	/* disable trace */
	write(trace_fd, "0", 1);
	close(trace_fd);
	close(marker_fd);

	return 0;
}
