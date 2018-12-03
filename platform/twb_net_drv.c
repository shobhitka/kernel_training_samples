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
#include <linux/interrupt.h>
#include <linux/ip.h>
#include <linux/workqueue.h>
#include "twb_net_dev.h"

static struct net_device *twbnet_dev_list[2];
static int dev_cnt = 0;

struct twb_pkt {
	struct net_device *ndev;
	struct net_device *dest;
	struct twb_pkt *next;
	u8 data[ETH_DATA_LEN];
	u16 len;
	struct work_struct rx_work;
};

struct twbnet_priv {
	struct net_device *dev;
	struct twbnet_platform_data *pdata;

	/* buffers */
	struct twb_pkt *pool;
	struct twb_pkt *rx_queue;
	u16 rx_queue_cnt;

	/* cache pointer to free later */
	struct sk_buff *skb;

	/* stats */
	u32 tx_cnt;
	u32 rx_cnt;
	u32 tx_dp_cnt;
	u32 rx_dp_cnt;

	u8 isr;

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

struct twb_pkt * twbnet_get_buffer(struct net_device *dev)
{
	struct twb_pkt *pkt;
	struct twbnet_priv *priv = netdev_priv(dev);

	if (mutex_lock_killable(&priv->mutex))
		return ERR_PTR(-EINTR);

	if (priv->pool) {
		pkt = priv->pool;
		priv->pool = priv->pool->next;
		pkt->next = NULL;
		pkt->ndev = dev;
	} else
		pkt = NULL;

	pkt->ndev = dev;

	mutex_unlock(&priv->mutex);
	return pkt;
}

void twbnet_put_buffer(struct twb_pkt *pkt)
{
	struct twbnet_priv *priv;
	struct twb_pkt *head;

	if (!pkt) {
		dev_err(&pkt->ndev->dev, "Null pkt to release !!\n");
		return;
	}

	priv = netdev_priv(pkt->ndev);
	head = priv->pool;

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

	if (netif_queue_stopped(pkt->ndev) && pkt->next == NULL)
		netif_wake_queue(pkt->ndev);
}

int twbnet_enqueue_rx(struct net_device *ndev, struct twb_pkt *pkt)
{
	struct twbnet_priv *priv = netdev_priv(ndev);

	if (mutex_lock_killable(&priv->mutex))
		return -EINTR;

	if (priv->rx_queue_cnt >= priv->pdata->hw_rx_size) {
		mutex_unlock(&priv->mutex);
		return -ENOMEM;
	}

	pkt->dest = ndev;

	if (priv->rx_queue)
		priv->rx_queue->next = pkt;
	else
		priv->rx_queue = pkt;

	priv->rx_queue_cnt++;

	mutex_unlock(&priv->mutex);

	return 0;
}

struct twb_pkt *twbnet_dequeue_rx(struct net_device *ndev)
{
	struct twb_pkt *pkt;
	struct twbnet_priv *priv = netdev_priv(ndev);

	if (mutex_lock_killable(&priv->mutex))
		return ERR_PTR(-EINTR);

	if (priv->rx_queue) {
		pkt = priv->rx_queue;
		priv->rx_queue = priv->rx_queue->next;
	} else
		pkt = NULL;

	priv->rx_queue_cnt--;
	mutex_unlock(&priv->mutex);
	return pkt;
}

static void twbnet_handle_rx(struct work_struct *work)
{
	struct sk_buff *skb;
	struct twb_pkt *pkt = container_of(work, struct twb_pkt, rx_work);
	struct twbnet_priv *priv = netdev_priv(pkt->dest);

	skb = dev_alloc_skb(pkt->len + 2);
	if (!skb) {
		priv->rx_dp_cnt++;
		goto out;
	}

	skb_reserve(skb, 2);
	memcpy(skb_put(skb, pkt->len), pkt->data, pkt->len);

	skb->dev = pkt->dest;
	skb->protocol = eth_type_trans(skb, pkt->dest);

	skb->ip_summed = CHECKSUM_UNNECESSARY;

	priv->rx_cnt++;

	netif_rx(skb);

	twbnet_put_buffer(pkt);
  out:
	return;
}

static irqreturn_t twbnet_irq_handler(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *) dev_id;
	struct twbnet_priv *priv = netdev_priv(dev);
	struct twb_pkt *pkt = NULL;


	if (priv->isr & INTR_RX) {
		priv->isr &= ~INTR_RX;
		pkt = twbnet_dequeue_rx(dev);
		if (pkt) {
			/* initialize work on deafult work queue */
			INIT_WORK(&pkt->rx_work, twbnet_handle_rx);
			schedule_work(&pkt->rx_work);
		}
	} else if (priv->isr & INTR_TX) {
		priv->isr &= ~INTR_TX;
		priv->tx_cnt++;
		dev_kfree_skb(priv->skb);
	}

	return IRQ_HANDLED;
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
	struct twbnet_priv *priv = netdev_priv(dev);
	struct twb_pkt *pkt;
	struct net_device *dest;
	struct twbnet_priv *dest_priv;
	struct iphdr *ih;
	u8 *data;
	u8 *saddr, *daddr;

	if (htons(skb->protocol) == 0x3 || htons(skb->protocol) == 0x86dd) {
		/* dev_info(&dev->dev, "Dropping packet. Protocol = 0x%x\n", htons(skb->protocol)); */
		dev_kfree_skb(skb);
		priv->tx_dp_cnt++;
		return NETDEV_TX_OK;
	}

	/* ensure that both the devices are probed */
	if (dev_cnt == 1) {
		dev_kfree_skb(skb);
		priv->tx_dp_cnt++;
		return NETDEV_TX_OK;
	}

	/* get a free packet from pool */
	pkt = twbnet_get_buffer(dev);
	if (!pkt) {
		// no free buffer drop this packet and stop the tx queue
		dev_kfree_skb(skb);
		netif_stop_queue(dev);
		priv->tx_dp_cnt++;
		return NETDEV_TX_OK;
	}

	/* save the skp being transmitted in the priv data so that
	 * we can free that after transmit */
	priv->skb = skb;

	pkt->len = skb->len;
	memcpy(pkt->data, skb->data, skb->len);

	/* need to modify the IP addresses so that they get routed correctly */
	data = pkt->data;
	ih = (struct iphdr *) (data + sizeof(struct ethhdr));
	saddr = (u8 *) &ih->saddr;
	daddr = (u8 *) &ih->daddr;

	if (saddr[2] == daddr[2]) {
		saddr[2] ^= 1;
		daddr[2] ^= 1;

		/* re-do the IP csum */
		ih->check = 0;
		ih->check = ip_fast_csum((unsigned char *)ih, ih->ihl);

		dev_info(&dev->dev, "%d.%d.%d.%d -> %d.%d.%d.%d\n", saddr[0], saddr[1], saddr[2], saddr[3],
				daddr[0], daddr[1], daddr[2], daddr[3]);
	}

	/* enqueu in rx_queue of destination interface */
	dest = twbnet_dev_list[twbnet_dev_list[0] == dev ? 1 : 0];
	twbnet_enqueue_rx(dest, pkt);

	/* raise interrupt to dest */
	dest_priv = netdev_priv(dest);
	dest_priv->isr |= INTR_RX;
	twbnet_irq_handler(TWBNET_IRQ_NUM, dest);

	priv->isr |= INTR_TX;
	twbnet_irq_handler(TWBNET_IRQ_NUM, dev);

	return NETDEV_TX_OK;
}

static const struct net_device_ops twbnet_ops = {
	.ndo_open       = twbnet_open,
	.ndo_stop       = twbnet_close,
	.ndo_start_xmit = twbnet_start_xmit,
};

static int twbnet_create_eth_hdr(struct sk_buff *skb, struct net_device *dev,
                unsigned short type, const void *daddr, const void *saddr,
                unsigned len)
{
	struct ethhdr *eth = (struct ethhdr *) skb_push(skb, ETH_HLEN);

	eth->h_proto = htons(type);
	memcpy(eth->h_source, saddr ? saddr : dev->dev_addr, dev->addr_len);
	memcpy(eth->h_dest,   daddr ? daddr : dev->dev_addr, dev->addr_len);

	eth->h_dest[ETH_ALEN-1] ^= 0x01;

	return (dev->hard_header_len);
}

static const struct header_ops twbnet_hdr_ops = {
	.create  = twbnet_create_eth_hdr,
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
	ndev->header_ops = &twbnet_hdr_ops;
	twb->dev = ndev;
	twb->tx_cnt = twb->rx_cnt = twb->tx_dp_cnt = twb->rx_dp_cnt = 0;

	pdata = (struct twbnet_platform_data *) pdev->dev.platform_data;
	twb->pdata = pdata;

	memcpy(ndev->dev_addr, pdata->mac, ETH_LEN);

	twb->pool = twbnet_init_pool(&pdev->dev, pdata->hw_pool_size);
	twb->rx_queue = NULL;
	twb->rx_queue_cnt = 0;
	mutex_init(&twb->mutex);

	/* indicate not to do ARP on thsi interface */
	ndev->flags |= IFF_NOARP;

	if ((retval = register_netdev (ndev))) {
		dev_err (&pdev->dev, "Error %d initializing network card", retval);
		return retval;
	}

	platform_set_drvdata(pdev, ndev);
	twbnet_dev_list[dev_cnt] = ndev;
	dev_cnt++;

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
