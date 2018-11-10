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
#include <linux/platform_device.h>

static struct resource twbnet_dev_resources[] = {
};

void twbnet_dev_release(struct device *dev)
{

}

static struct platform_device twbnet_dev = {
	.name = "twbnet",
	.id = 1,
	.num_resources = ARRAY_SIZE(twbnet_dev_resources),
	.resource = twbnet_dev_resources,
	.dev = {
		.release = twbnet_dev_release,
	},
};

int twbnet_dev_init(void)
{
	printk ("[twbnet]: Adding new platform device\n");

	platform_device_register(&twbnet_dev);
	return 0;
}

void twbnet_dev_cleanup (void)
{
	printk ("[twbnet]: Releasing device\n");
	platform_device_unregister(&twbnet_dev);
	return;
}

module_init (twbnet_dev_init);
module_exit (twbnet_dev_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Pseudo Network Device");
MODULE_AUTHOR("Shobhit Kumar");
