#ifndef __TWB_IOCTL_H__
#define __TWB_IOCTL_H__

#include <linux/ioctl.h>

#define TWBMEM_MAGIC 		236
#define TWBMEM_GETBUFFSIZE 	_IOR(TWBMEM_MAGIC, 1, int *)
#define TWBMEM_SETBUFFSIZE 	_IOW(TWBMEM_MAGIC, 2, int *)
#define TWBMEM_MAX_IOCTL 	2

#endif /* TWB_IOCTL_H__ */
