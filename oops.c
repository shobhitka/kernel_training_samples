#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

static void generate_oops(void)
{
	*(int *) 0 = 0;
}

static int __init twb_oops_init(void)
{
	printk("[twb] OOPs from the module\n");
	generate_oops();
	return (0);
}

static void __exit twb_oops_exit(void)
{
	printk("[twb] Goodbye world\n");
}

module_init(twb_oops_init);
module_exit(twb_oops_exit);
