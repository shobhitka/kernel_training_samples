#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time.h>

static char *who = "World";
module_param(who, charp, S_IRUGO|S_IWUSR);

struct timeval tv1, tv2;

static int __init hello_init(void)
{
	do_gettimeofday(&tv1);
	printk("Hello %s\n", who);
	return 0;
}

static void __exit hello_exit(void)
{
	do_gettimeofday(&tv2);
	printk("Bye bye %s. Time loaded = %ld seconds\n", who, tv2.tv_sec - tv1.tv_sec);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hello World");
MODULE_AUTHOR("Shobhit Kumar");
