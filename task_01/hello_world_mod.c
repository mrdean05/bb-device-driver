#include <linux/module.h>

static int __init hello_world_init(void)
{
	pr_info("Hello World\n");
	return 0;
}

static void __exit hello_world_exit(void)
{
	pr_info("Bye bye everyone\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("mr_dean");
MODULE_DESCRIPTION("This is a hello word module");
MODULE_INFO(board,"Beaglebone black");
