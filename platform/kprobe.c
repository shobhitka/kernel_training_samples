#include<linux/module.h>
#include<linux/version.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/kprobes.h>

static char *symbol = "twbnet_start_xmit";
module_param(symbol, charp, S_IRUGO|S_IWUSR);

int kp_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	printk("[twb-probe][%s][0x%p] Pre-Handler: IP: 0x%lx, FLAGS: 0x%lx\n", p->symbol_name, p->addr, regs->ip, regs->flags);

//	dump_stack();
	return 0;
}

void kp_post_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
	printk("[twb-probe][%s][0x%p] Post Handler: IP: 0x%lx, FLAGS: 0x%lx\n", p->symbol_name, p->addr, regs->ip, regs->flags);
}

static struct kprobe kp;

int twb_kprobe_init(void)
{
	kp.pre_handler = kp_pre_handler;
	kp.post_handler = kp_post_handler;
	kp.symbol_name = symbol;
	register_kprobe(&kp);
	return 0;
}

void twb_kprobe_exit(void)
{
	unregister_kprobe(&kp);
}

module_init(twb_kprobe_init);
module_exit(twb_kprobe_exit);
MODULE_LICENSE("GPL");
