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
#include <linux/netdevice.h> 
#include <linux/etherdevice.h> 

struct net_device *twbnet;

struct twbnet_priv {
	struct net_device *dev;
	u32 tx_cnt;
};

int twbnet_open(struct net_device *dev)
{
	netif_start_queue (dev);
	return 0;
}

int twbnet_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

static int twbnet_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	printk ("[twbnet]: Sending data\n");

	dev_kfree_skb(skb);
	return 0;
}

static const struct net_device_ops twbnet_ops = {
	.ndo_open       = twbnet_open,
	.ndo_stop       = twbnet_close,
	.ndo_start_xmit = twbnet_start_xmit,
};

int twbnet_drv_probe(struct platform_device *pdev)
{
	int retval;
	struct twbnet_priv *twb;
	struct net_device *ndev;
	
	printk ("[twbnet]: Probing net device\n");

	ndev = devm_alloc_etherdev(&pdev->dev, sizeof(*twb));
	if (!ndev)
		return -ENOMEM;

	SET_NETDEV_DEV(ndev, &pdev->dev);

	twb = netdev_priv(ndev);

	ndev->netdev_ops = &twbnet_ops;
	twb->dev = ndev;
	twb->tx_cnt = 0;

	if ((retval = register_netdev (ndev))) {
		printk ("[twbnet] Error %d initializing network card", retval);
		return retval;
	}

	twbnet = ndev;
	return 0;
}

int twbnet_drv_remove(struct platform_device *pdev)
{
	printk ("[twbnet]: Removing net device\n");
	unregister_netdev(twbnet);
	return 0;
}

static struct platform_driver twbnet_drv = {
	.probe = twbnet_drv_probe,
	.remove = twbnet_drv_remove,
	.driver = {
		.name = "twbnet",
	},
};

int twbnet_drv_init(void)
{
	printk("[twbnet]: Registering platform driver\n");
	platform_driver_register(&twbnet_drv);

	return 0;
}

void twbnet_drv_cleanup(void)
{
	printk("[twbnet]: Unloading platform driver\n");
	platform_driver_unregister(&twbnet_drv);
}

module_init (twbnet_drv_init);
module_exit (twbnet_drv_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Pseudo Network Driver");
MODULE_AUTHOR("Shobhit Kumar");
