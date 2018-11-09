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

#define DRIVERNAME 	"twbnet"
#define BUSNAME 	"dummy"
#define VERSION 	"1.0"

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
	const char *type;

	switch(skb->pkt_type) {
		case PACKET_HOST:
			type = "PACKET_HOST";
			break;
		case PACKET_BROADCAST:
			type = "PACKET_BROADCAST";
			break;
		case PACKET_MULTICAST:
			type = "PACKET_MULTICAST";
			break;
		case PACKET_OTHERHOST:
			type = "PACKET_OTHERHOST";
			break;
		case PACKET_OUTGOING:
			type = "PACKET_OUTGOING";
			break;
		case PACKET_LOOPBACK:
			type = "PACKET_LOOPBACK";
			break;
		case PACKET_USER:
			type = "PACKET_USER";
			break;
		case PACKET_KERNEL:
			type = "PACKET_KERNEL";
			break;
		default:
			type = "UNKNOWN";
			break;
	}

	printk ("[twbnet]: Dropping packet. Type: %s, Protocol = 0x%x\n", type, cpu_to_be16(skb->protocol));

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

static void twbnet_ethtool_get_drvinfo(struct net_device *dev,
								struct ethtool_drvinfo *info)
{
	strlcpy(info->driver, DRIVERNAME, sizeof(info->driver));
	strlcpy(info->bus_info, BUSNAME, sizeof(info->bus_info));
	strlcpy(info->version, VERSION, sizeof(info->version));
}

static const struct ethtool_ops twbnet_ethtool_ops = {
	.get_drvinfo = twbnet_ethtool_get_drvinfo,
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
	ndev->ethtool_ops = &twbnet_ethtool_ops;
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
