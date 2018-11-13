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
#include "twb_net_dev.h"

static struct twbnet_platform_data twbnet_data_1 = {
	.mac = {0xde, 0xad, 0xbe, 0xaf, 0x00, 0x00},
	.hw_rx_size = 16,
	.hw_pool_size = 16,
};

static struct twbnet_platform_data twbnet_data_2 = {
	.mac = {0xde, 0xad, 0xbe, 0xaf, 0x00, 0x01},
	.hw_rx_size = 16,
	.hw_pool_size = 16,
};

static struct resource twbnet_dev_resources[] = {
};

void twbnet_dev_release(struct device *dev)
{

}

static struct platform_device twbnet_dev_1 = {
	.name = "twbnet",
	.id = -2,
	.num_resources = ARRAY_SIZE(twbnet_dev_resources),
	.resource = twbnet_dev_resources,
	.dev = {
		.release = twbnet_dev_release,
		.platform_data = &twbnet_data_1,
	},
};

static struct platform_device twbnet_dev_2 = {
	.name = "twbnet",
	.id = -2,
	.num_resources = ARRAY_SIZE(twbnet_dev_resources),
	.resource = twbnet_dev_resources,
	.dev = {
		.release = twbnet_dev_release,
		.platform_data = &twbnet_data_2,
	},
};

int twbnet_dev_init(void)
{
	printk ("[twbnet]: Adding new platform devices\n");

	platform_device_register(&twbnet_dev_1);
	platform_device_register(&twbnet_dev_2);
	return 0;
}

void twbnet_dev_cleanup (void)
{
	printk ("[twbnet]: Releasing devices");
	platform_device_unregister(&twbnet_dev_1);
	platform_device_unregister(&twbnet_dev_2);
	return;
}

module_init (twbnet_dev_init);
module_exit (twbnet_dev_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Pseudo Network Device");
MODULE_AUTHOR("Shobhit Kumar");
