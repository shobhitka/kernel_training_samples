#ifndef __TWB_NET_DEV_H__
#define __TWB_NET_DEV_H__

#define ETH_LEN 	6

struct twbnet_platform_data {
	u8 mac[ETH_LEN];
	u8 hw_rx_size;
	u8 hw_pool_size;
};

#endif /* __TWB_NET_DEV_H__ */
