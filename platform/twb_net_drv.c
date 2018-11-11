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
#include "twb_net_dev.h"


static struct net_device *twbnet_dev_list[2];
static int dev_cnt = 0;

struct twb_pkt {
	struct net_device *ndev;
	struct twb_pkt *next;
	u8 *data;
	u16 len;
};

struct twbnet_priv {
	struct net_device *dev;
	struct twbnet_platform_data *pdata;

	/* buffers */
	struct twb_pkt *pool;
	struct twb_pkt *rx_queue;
	u16 rx_queue_cnt;

	/* stats */
	u32 tx_cnt;
	u32 rx_cnt;
	u32 dp_cnt;

	struct mutex mutex;
};

struct twb_pkt * twbnet_init_pool(struct device *dev, int num)
{
	struct twb_pkt *head = NULL;
	int i;

	for (i = num; i > 0; i--) {
		struct twb_pkt *pkt = devm_kzalloc(dev, sizeof(struct twb_pkt), GFP_KERNEL);
		if (!pkt)
			return NULL;

		if (head) {
			pkt->next = head;
			head = pkt;
		} else {
			head = pkt;
			pkt->next = NULL;
		}
	}

	return head;
}

struct twb_pkt * twbnet_get_buffer(struct twbnet_priv *priv)
{
	struct twb_pkt *pkt;

	if (mutex_lock_killable(&priv->mutex))
		return ERR_PTR(-EINTR);

	if (priv->pool) {
		pkt = priv->pool;
		priv->pool = priv->pool->next;
	} else
		pkt = NULL;

	mutex_unlock(&priv->mutex);
	return pkt;
}

void twbnet_put_buffer(struct twbnet_priv *priv, struct twb_pkt *pkt)
{
	struct twb_pkt *head = priv->pool;

	if (mutex_lock_killable(&priv->mutex))
		return;

	if (head) {
		pkt->next = head;
		priv->pool = pkt;
	} else {
		priv->pool = pkt;
		pkt->next = NULL;
	}

	mutex_unlock(&priv->mutex);
}

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
	dev_info (&dev->dev, "Sending data\n");

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
	struct twbnet_platform_data *pdata;
	
	dev_info (&pdev->dev, "Probing net device\n");

	ndev = devm_alloc_etherdev(&pdev->dev, sizeof(*twb));
	if (!ndev)
		return -ENOMEM;

	SET_NETDEV_DEV(ndev, &pdev->dev);

	twb = netdev_priv(ndev);

	ndev->netdev_ops = &twbnet_ops;
	twb->dev = ndev;
	twb->tx_cnt = twb->rx_cnt = twb->dp_cnt = 0;

	pdata = (struct twbnet_platform_data *) pdev->dev.platform_data;
	twb->pdata = pdata;

	memcpy(ndev->dev_addr, pdata->mac, ETH_LEN);

	twb->pool = twbnet_init_pool(&pdev->dev, pdata->hw_pool_size);
	twb->rx_queue = NULL;
	twb->rx_queue_cnt = 0;
	mutex_init(&twb->mutex);

	if ((retval = register_netdev (ndev))) {
		dev_err (&pdev->dev, "Error %d initializing network card", retval);
		return retval;
	}

	platform_set_drvdata(pdev, ndev);
	twbnet_dev_list[dev_cnt++] = ndev;
	return 0;
}

int twbnet_drv_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);

	dev_info (&pdev->dev, "Removing net device\n");

	unregister_netdev(dev);
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
