#ifndef __TWB_MEM_H__
#define __TWB_MEM_H__

#define TWB_MEM_NUM_DEVICES 2

/* Size of the global memory buffer */
#define TWB_MEM_BUFFER_SIZE 1024

#define TWB_MEM_DEVICE_NAME 	"twbmem"

struct twb_mem_dev {
	unsigned char *data;
	unsigned long size;
	unsigned long curr_size;
	struct mutex mem_mutex;
	wait_queue_head_t queue;
	struct cdev cdev;
};
#endif /* __TWB_MEM_H__ */
