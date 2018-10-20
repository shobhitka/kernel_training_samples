#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static char *who = "World";
module_param(who, charp, S_IRUGO|S_IWUSR);

static int __init hello_init(void)
{
	printk("Hello %s\n", who);
	return 0;
}

static void __exit hello_exit(void)
{
	printk("Bye bye %s\n", who);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hello World");
MODULE_AUTHOR("Shobhit Kumar");
