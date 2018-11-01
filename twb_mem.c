#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include "twb_mem.h"
#include "twb_ioctl.h"

static int buf_size = TWB_MEM_BUFFER_SIZE;
module_param(buf_size, int, S_IRUGO|S_IWUSR);

static int ndevices = TWB_MEM_NUM_DEVICES;
module_param(ndevices, int, S_IRUGO|S_IWUSR);

static unsigned int twb_mem_major = 0;
static struct twb_mem_dev *mem_devices = NULL;
static struct class *twb_mem_class = NULL;

int
twb_mem_open(struct inode *inode, struct file *filp)
{
	unsigned int major = imajor(inode);
	unsigned int minor = iminor(inode);

	struct twb_mem_dev *dev = NULL;

	if (major != twb_mem_major || minor < 0 || minor >= ndevices)
	{
		printk(KERN_WARNING "[twb-mem] No device found with minor=%d and major=%d\n", major, minor);
		return -ENODEV;
	}

	/* store a pointer to struct twb_mem_dev here for other methods */
	dev = &mem_devices[minor];
	filp->private_data = dev;

	if (inode->i_cdev != &dev->cdev)
	{
		printk(KERN_WARNING "[twb-mem] open: internal error\n");
		return -ENODEV;
	}

	/* allocate the data buffer only once */
	if (dev->data == NULL)
	{
		dev->data = (unsigned char *) kzalloc(dev->size, GFP_KERNEL);
		if (dev->data == NULL)
		{
			printk(KERN_WARNING "[twb-mem] open(): out of memory\n");
			return -ENOMEM;
		}

		dev->curr_size = 0;

		// initialize per device wait queue
		init_waitqueue_head(&dev->queue);
	}

	return 0;
}

int
twb_mem_release(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t
twb_mem_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct twb_mem_dev *dev = (struct twb_mem_dev *) filp->private_data;
	ssize_t retval = 0;

	if (mutex_lock_killable(&dev->mem_mutex))
		return -EINTR;

	if (dev->curr_size == 0) {
		mutex_unlock(&dev->mem_mutex);
		wait_event_interruptible(dev->queue, dev->curr_size != 0);
		if (mutex_lock_killable(&dev->mem_mutex))
			return -EINTR;
	}

	if (*f_pos >= dev->curr_size) { /* EOF */
		// all data read by somebody, we can clear the buffer,
		// only for dmeonstation purposes with a tool like cat'
		// that reads all data from a file till EOF is received
		// Proper implementation would be to empty only the part
		// that is read. For now empty only at EOF
		dev->curr_size = 0;
		memset(dev->data, 0, dev->size);
		goto out;
	}

	if (*f_pos + count > dev->curr_size)
		count = dev->curr_size - *f_pos;

	if (copy_to_user(buf, &(dev->data[*f_pos]), count) != 0) {
		retval = -EFAULT;
		goto out;
	}

	*f_pos += count;
	retval = count;
out:
	mutex_unlock(&dev->mem_mutex);
	return retval;
}

ssize_t
twb_mem_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct twb_mem_dev *dev = (struct twb_mem_dev *) filp->private_data;
	ssize_t retval = 0;

	if (mutex_lock_killable(&dev->mem_mutex))
		return -EINTR;

	if (*f_pos >= dev->size) {
		retval = -EINVAL;
		goto out;
	}

	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	if (copy_from_user(&(dev->data[*f_pos]), buf, count) != 0) {
		retval = -EFAULT;
		goto out;
	}

	dev->curr_size += count;
	*f_pos += count;
	retval = count;

out:
	mutex_unlock(&dev->mem_mutex);

	/* wakeup all threads waiting for buffer to be ready */
	wake_up_interruptible(&dev->queue);

	return retval;
}

loff_t
twb_mem_llseek(struct file *filp, loff_t off, int whence)
{
	struct twb_mem_dev *dev = (struct twb_mem_dev *) filp->private_data;
	loff_t newpos = 0;

	switch(whence) {
	  case 0: /* SEEK_SET */
		newpos = off;
		break;

	  case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;

	  case 2: /* SEEK_END */
		newpos = dev->size + off;
		break;

	  default: /* can't happen */
		return -EINVAL;
	}

	if (newpos < 0 || newpos > dev->size)
		return -EINVAL;

	filp->f_pos = newpos;
	return newpos;
}


long twb_mem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned long int new_val;
	bool err = false;
	struct twb_mem_dev *dev = (struct twb_mem_dev *) filp->private_data;

	if (_IOC_TYPE(cmd) != TWBMEM_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > TWBMEM_MAX_IOCTL) return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void *) arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void *) arg, _IOC_SIZE(cmd));

	if (err)
		return -EFAULT;

	printk("[twb-mem] called twb_mem_ioctl, number: %d\n", _IOC_NR(cmd));

	switch (cmd) {
		case TWBMEM_GETBUFFSIZE:
			copy_to_user((int *) arg, &dev->size, sizeof(dev->size));
			break;
		case TWBMEM_SETBUFFSIZE:
			copy_from_user(&new_val, (int *) arg, sizeof(dev->size));

			/* we need to reallocate the device buffer memory */
			if (new_val == 0) {
				printk("[twb-mem] Are you trying to free the memory ? Ignoring this request\n");
				return -EINVAL;
			}

			if (new_val == dev->size) {
				printk("[twb-mem] New size is same as existing size.  Ignoring this request\n");
				return 0;
			}

			dev->data = krealloc(dev->data, new_val, GFP_KERNEL);
			if (!dev->data) {
				printk("[twb-mem] Cannot allocate memory\n");
				return -ENOMEM;
			}

			dev->size = new_val;
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

struct file_operations twb_mem_fops = {
	.owner =    THIS_MODULE,
	.read =     twb_mem_read,
	.write =    twb_mem_write,
	.open =     twb_mem_open,
	.release =  twb_mem_release,
	.llseek =   twb_mem_llseek,
	.unlocked_ioctl = twb_mem_ioctl,
};

static int
twb_mem_create_device(struct twb_mem_dev *dev, int minor, struct class *class)
{
	int err = 0;
	dev_t devno = MKDEV(twb_mem_major, minor);
	struct device *device = NULL;

	/* Memory is to be allocated when the device is opened the first time */
	dev->data = NULL;
	dev->size = buf_size;
	mutex_init(&dev->mem_mutex);

	cdev_init(&dev->cdev, &twb_mem_fops);
	dev->cdev.owner = THIS_MODULE;

	err = cdev_add(&dev->cdev, devno, 1);
	if (err) {
		printk(KERN_WARNING "[twb-mem] Error %d while trying to add %s%d", err, TWB_MEM_DEVICE_NAME, minor);
		return err;
	}

	device = device_create(class, NULL, /* no parent device */
					devno, NULL, /* no additional data */
					TWB_MEM_DEVICE_NAME "%d", minor);

	if (IS_ERR(device)) {
		err = PTR_ERR(device);
		printk(KERN_WARNING "[twb-mem] Error %d while trying to create %s%d", err, TWB_MEM_DEVICE_NAME, minor);
		cdev_del(&dev->cdev);
		return err;
	}
	return 0;
}

static void
twb_mem_cleanup(int index)
{
	int i;

	if (mem_devices) {
		for (i = 0; i < index; ++i) {
			struct twb_mem_dev *dev = &mem_devices[i];
			device_destroy(twb_mem_class, MKDEV(twb_mem_major, i));
			cdev_del(&dev->cdev);
			kfree(dev->data);
			mutex_destroy(&dev->mem_mutex);
		}

		kfree(mem_devices);
	}

	class_destroy(twb_mem_class);

	unregister_chrdev_region(MKDEV(twb_mem_major, 0), ndevices);
	return;
}

static int __init twb_mem_init(void)
{
	int err = 0;
	int i = 0;
	int count = 0;
	dev_t dev = 0;

	if (ndevices <= 0) {
		printk(KERN_WARNING "[twb_mem] No devices requested: %d\n", ndevices);
		err = -EINVAL;
		return err;
	}

	/* Get a range of minor numbers */
	err = alloc_chrdev_region(&dev, 0, ndevices, TWB_MEM_DEVICE_NAME);
	if (err < 0) {
		printk(KERN_WARNING "[twb-mem] alloc_chrdev_region() failed\n");
		return err;
	}

	twb_mem_major = MAJOR(dev);

	/* Create device class */
	twb_mem_class = class_create(THIS_MODULE, TWB_MEM_DEVICE_NAME);
	if (IS_ERR(twb_mem_class)) {
		err = PTR_ERR(twb_mem_class);
		goto fail;
	}

	/* Allocate the devices */
	mem_devices = (struct twb_mem_dev *) kzalloc(ndevices * sizeof(struct twb_mem_dev), GFP_KERNEL);
	if (mem_devices == NULL) {
		err = -ENOMEM;
		goto fail;
	}

	/* create devices */
	for (i = 0; i < ndevices; ++i) {
		err = twb_mem_create_device(&mem_devices[i], i, twb_mem_class);
		if (err) {
			count = i;
			goto fail;
		}
	}
	return 0;

fail:
	twb_mem_cleanup(count);
	return err;
}

static void __exit twb_mem_exit(void)
{
	twb_mem_cleanup(ndevices);
}

module_init(twb_mem_init);
module_exit(twb_mem_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Memory Based Charachter Driver");
MODULE_AUTHOR("Shobhit Kumar");
