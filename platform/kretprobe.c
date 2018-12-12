#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/ktime.h>

static char symbol_name[NAME_MAX] = "twbnet_start_xmit";
module_param_string(symbol, symbol_name, NAME_MAX, S_IRUGO);

/* private data */
struct twb_kret_data {
	ktime_t timestamp;
};

static int twb_entry_handler(struct kretprobe_instance *kret, struct pt_regs *regs)
{
	struct twb_kret_data *data = (struct twb_kret_data *) kret->data;

	data->timestamp = ktime_get();
	return 0;
}

static int twb_ret_handler(struct kretprobe_instance *kret, struct pt_regs *regs)
{
	s64 delta;
	struct twb_kret_data *data = (struct twb_kret_data *) kret->data;
	unsigned long retval = regs_return_value(regs);
	ktime_t now = ktime_get();

	delta = ktime_to_ns(ktime_sub(now, data->timestamp));

	pr_info("[twb-kret][%s][0x%p] Returned value from the symbol: %lu and it took %lld ns to complete\n",
			kret->rp->kp.symbol_name, kret->rp->kp.addr, retval, (long long) delta);

	return 0;
}

static struct kretprobe twb_kret = {
	.handler		= twb_ret_handler,
	.entry_handler	= twb_entry_handler,
	.data_size		= sizeof(struct twb_kret_data),
	.maxactive		= 5,
};

static int __init twb_kret_init(void)
{
	int ret;

	twb_kret.kp.symbol_name = symbol_name;
	ret = register_kretprobe(&twb_kret);
	if (ret < 0) {
		pr_err("[twb-kret][%s] Registering kretprobe failed, err = %d\n", symbol_name, ret);
		return -1;
	}

	pr_info("[twb-kret][%s][0x%p] Kretprobe set succesfully\n",
			twb_kret.kp.symbol_name, twb_kret.kp.addr);
	return 0;
}

static void __exit twb_kret_exit(void)
{
	unregister_kretprobe(&twb_kret);
	pr_info("[twb-kret][%s][0x%p] Kretprobe unregistered\n",
			twb_kret.kp.symbol_name, twb_kret.kp.addr);

	pr_info("[twb-kret][%s][0x%p] Missed count = %lu\n",
			twb_kret.kp.symbol_name, twb_kret.kp.addr, twb_kret.kp.nmissed);
}

module_init(twb_kret_init)
module_exit(twb_kret_exit)
MODULE_LICENSE("GPL");
