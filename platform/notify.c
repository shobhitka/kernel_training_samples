#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/netdevice.h>


int notify_handler(struct notifier_block *self, unsigned long action, void *dev)
{
	char *act_str;
	switch (action) {
		case NETDEV_UP:
			act_str = "NETDEV_UP";
			break;
		case NETDEV_DOWN:
			act_str = "NETDEV_DOWN";
			break;
		case NETDEV_CHANGE:
			act_str = "NETDEV_CHANGE";
			break;
		case NETDEV_REGISTER:
			act_str = "NETDEV_REGISTER";
			break;
		case NETDEV_UNREGISTER:
			act_str = "NETDEV_UNREGISTER";
			break;
		default:
			act_str = "NOT_PARSED";
			break;
	}

	pr_info("[twb-notify] Netdev Notification: %s\n", act_str);
	return NOTIFY_OK;
}

struct notifier_block nb = {
	.notifier_call = notify_handler,
};

static int __init twb_notify_init(void)
{
	register_netdevice_notifier(&nb);

	return 0;
}

static void __exit twb_notify_exit(void)
{
	unregister_netdevice_notifier(&nb);
}

module_init(twb_notify_init);
module_exit(twb_notify_exit);

MODULE_LICENSE("GPL");
