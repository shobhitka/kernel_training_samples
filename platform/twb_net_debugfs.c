#include <linux/debugfs.h>
#include <linux/device.h>
#include "twb_net_dev.h"
#include "twb_net_drv.h"

static int twbnet_rxqueue_get(void *data, u64 *val)
{
	struct twbnet_priv *priv = (struct twbnet_priv *) data;

	*val = priv->pdata->hw_rx_size;
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(twbnet_rxqueue_fops, twbnet_rxqueue_get, NULL, "%llu\n");

static int twbnet_queue_show(struct seq_file *m, void *data)
{
	struct twbnet_priv *priv = (struct twbnet_priv *) m->private;

	/* give all the queue related data */
	seq_printf(m, "HW_TX_POOL_SIZE: %d\n", priv->pdata->hw_pool_size);
	seq_printf(m, "HW_RX_QUEUE_SIZE: %d\n", priv->pdata->hw_rx_size);
	seq_printf(m, "HW_RX_QUEUE_CURRENT_COUNT: %d\n", priv->rx_queue_cnt);
	seq_printf(m, "HW_RX_COUNT: %d\n", priv->rx_cnt);
	seq_printf(m, "HW_RX_DP_COUNT: %d\n", priv->rx_dp_cnt);
	seq_printf(m, "HW_TX_COUNT: %d\n", priv->tx_cnt);
	seq_printf(m, "HW_TX_DP_COUNT: %d\n", priv->tx_dp_cnt);

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(twbnet_queue);

int twbnet_setup_debugfs(struct twbnet_priv *priv)
{
	struct dentry *ent;

	priv->debugfs_root = debugfs_create_dir(priv->name, NULL);
	if (!priv->debugfs_root)
		return -ENOMEM;

	ent = debugfs_create_file("rx_queue_size", S_IRUGO, priv->debugfs_root, (void *) priv, &twbnet_rxqueue_fops);
	if (!ent)
		return -ENOMEM;

	ent = debugfs_create_file("queues", S_IRUGO, priv->debugfs_root, (void *) priv, &twbnet_queue_fops);
	if (!ent)
		return -ENOMEM;

	return 0;
}

void twbnet_tear_debugfs(struct twbnet_priv *priv)
{
	debugfs_remove_recursive(priv->debugfs_root);
}
