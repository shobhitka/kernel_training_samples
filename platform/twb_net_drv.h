#ifndef __TWB_NET_DRV_H__
#define __TWB_NET_DRV_H__

#include <linux/netdevice.h>
#include <linux/etherdevice.h>

struct twb_pkt {
	struct net_device *ndev;
	struct net_device *dest;
	struct twb_pkt *next;
	u8 data[ETH_DATA_LEN];
	u16 len;
#ifdef USE_TASKLET
	struct tasklet_struct tasklet;
#else
	struct work_struct rx_work;
#endif
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

extern void twbnet_setup_sysfs(struct twbnet_priv *priv);
extern void twbnet_tear_sysfs(struct twbnet_priv *priv);

#endif /* __TWB_NET_DRV_H__ */
