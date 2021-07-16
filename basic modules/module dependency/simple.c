//hello.c module dependency
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
int xvar=100;

void sayHello(void) { }
static int __init simple_init(void)
{
	printk("Hello World.. welcome \nModule dependency\n");
	return 0;
}
static void __exit simple_exit(void)
{
	printk("Bye, Leaving the world\n");
}

module_init(simple_init);
module_exit(simple_exit);
EXPORT_SYMBOL_GPL(xvar);
EXPORT_SYMBOL_GPL(sayHello);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arrijith");
MODULE_DESCRIPTION("Module dependency - hello world");
