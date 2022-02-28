#include <linux/init.h>
#include <linux/module.h>

static int __init hello_init(void)
{
	printk("hello init\r\n");
	return 0;
}

static void __exit  hello_exit(void)
{
	printk("hello exit\r\n");

}


module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
