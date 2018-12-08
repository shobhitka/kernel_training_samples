#include <linux/device.h>
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/sysfs.h>
#include "twb_net_drv.h"

static ssize_t rx_queue_status_show(struct device *kdev, struct device_attribute *attr, char *buf)
{
	struct net_device *ndev = container_of(kdev, struct net_device, dev);
	struct twbnet_priv *priv = netdev_priv(ndev);

	return snprintf(buf, PAGE_SIZE, "%d\n", priv->rx_queue_cnt);
}

/* define the atrributes */
static DEVICE_ATTR(rx_queue_status, S_IRUGO, rx_queue_status_show, NULL);

void twbnet_setup_sysfs(struct twbnet_priv *priv)
{
	int ret;
	struct device *kdev = &priv->dev->dev;

	ret = sysfs_create_file(&kdev->kobj, &dev_attr_rx_queue_status.attr);
	if (ret)
		dev_err(kdev, "Failed to setup sysfs attributes\n");
}

void twbnet_tear_sysfs(struct twbnet_priv *priv)
{
	struct device *kdev = &priv->dev->dev;

	sysfs_remove_file(&kdev->kobj, &dev_attr_rx_queue_status.attr);
}
