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
	struct net_device_stats *stats;

	printk ("[twbnet]: Sending data\n");

	stats = &dev->stats;
	stats->tx_dropped++;

	dev_kfree_skb(skb);
	return 0;
}

static const struct net_device_ops twbnet_ops = {
	.ndo_open       = twbnet_open,
	.ndo_stop       = twbnet_close,
	.ndo_start_xmit = twbnet_start_xmit,
};

int twbnet_init(void)
{
	int retval;
	struct twbnet_priv *twb;

	struct net_device *ndev = alloc_etherdev(sizeof(*twb));
	if (!ndev)
		return -ENOMEM;

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

void twbnet_cleanup (void)
{
	printk ("[twbnet]: Releasing device\n");
	unregister_netdev(twbnet);
	return;
}

module_init (twbnet_init);
module_exit (twbnet_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Pseudo Network Driver");
MODULE_AUTHOR("Shobhit Kumar");
